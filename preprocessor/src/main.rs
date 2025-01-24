pub mod module_loader;
mod scheduler;
mod database;
mod fac_id;

use std::{collections::HashMap, os::unix::net::UnixStream};
use std::io::prelude::*;
use mysql::prelude::*;
use std::ffi::{CStr, CString};
use self::scheduler::scheduler::Scheduler;
use chrono::{DateTime, Duration, Local, Timelike};
use std::sync::{Mutex, Arc};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let dberr = database::database::connect();
    let mut scheduler = Arc::new(Mutex::new(Scheduler::new()));

    if dberr.is_err() {
        return dberr;
        // println!("Failed to connect to database: {}", dberr.err().unwrap());
    }

    // Load modules
    let mut nmods = 0;
    let m = unsafe { std::slice::from_raw_parts(module_loader::load_modules(&mut nmods), nmods as usize) };

    let mut conn = database::database::get_pool().unwrap().get_conn().unwrap();

    unsafe {
        for module in m {
            let mut meta = *module_loader::get_module_meta(module.id);
            meta.facility_id = fac_id::generate_facility_id(&meta.facility_address, &meta.facility_name, meta.facility_cap);

            let str_fac_name = CStr::from_bytes_until_nul(&meta.facility_name)?.to_str().unwrap();
            let str_fac_addr = CStr::from_bytes_until_nul(&meta.facility_address)?.to_str().unwrap();

            let res = conn.exec_drop("INSERT INTO facilities (Name, Address, ID, Capacity, StateCode) VALUES(?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE Capacity = VALUES(Capacity)", (str_fac_name, str_fac_addr, meta.facility_id, meta.facility_cap, &meta.state_code[0..2]));

            if res.is_err() {
                println!("Failed to insert {} into the facilities database: {}", str_fac_name, res.err().unwrap());
                continue;
            }

            let mut t: DateTime<Local> = Local::now();
            let mins: u8 = t.minute().try_into().unwrap();
            if  mins >= meta.start_time {
                t = t + Duration::hours(1);
                t = t - Duration::minutes((mins - meta.start_time).into());
            } else {
                t = t + Duration::minutes((meta.start_time - mins).into());
            }

            if meta.is_incremental != 0 {
                let m_id = module.id;
                let a_sch = Arc::clone(&scheduler);
                scheduler.lock().unwrap().push_interval_job(Local::now(), meta.run_interval, move || {
                    let mut results: Arc<Mutex<Vec<module_loader::ModuleOut>>> = Arc::new(Mutex::new(Vec::new()));
                    let mut cache: Arc<Mutex<HashMap<String, module_loader::ModuleOut>>> = Arc::new(Mutex::new(HashMap::new()));

                    let mut initial_out = module_loader::ModuleOut {
                        enc: 0,
                        out: std::ptr::null()
                    };

                    module_loader::exec_module(m_id, &mut initial_out);
                    
                    let json = serde_json::from_str(CStr::from_ptr(initial_out.out).to_str().unwrap());
                    if json.is_err() {
                        return;
                    }

                    let obj = json.unwrap();

                    if let serde_json::Value::Object(elements) = obj {
                        let mut t = Local::now();
                        let inc = Duration::seconds(4);
                        for key in elements.keys() {
                            if cache.lock().unwrap().get(key).is_some() {
                                continue;
                            }

                            let s = Box::new(key.clone());
                            // a_sch.lock().unwrap().push_job(t, move || {
                                let mut out = module_loader::ModuleOut {
                                    enc: 0,
                                    out: std::ptr::null(),
                                };

                                let cstr = CString::new(s.as_str()).unwrap();
                                module_loader::exec_module_inc(cstr.as_ptr(), m_id, &mut out);

                                println!("{}", CStr::from_ptr(out.out).to_str().unwrap());
                            // });

                            t = t + inc;
                        }
                    }

                });
            } else {
                let meta_cp = Arc::new(meta);
                let m_id = module.id;
                scheduler.lock().unwrap().push_interval_job(t, meta.run_interval, move || {
                    let mut stream = UnixStream::connect("/tmp/sniffy.socket").unwrap();
                    stream.set_nonblocking(false).expect("Failed to set socket to blocking");
                    let mut out = module_loader::ModuleOut {
                        enc: 0,
                        out: std::ptr::null(),
                    };
                    println!("Executing module {} Fac ID: {}-{:08X}", CStr::from_bytes_until_nul(&meta_cp.facility_name).unwrap().to_str().unwrap(), CStr::from_bytes_until_nul(&meta_cp.state_code).unwrap().to_str().unwrap(), meta_cp.facility_id);
                    module_loader::exec_module(m_id, &mut out);

                    let mut str = String::from(std::ffi::CStr::from_ptr(out.out).to_str().unwrap());
                    str = str.to_lowercase();
                    let json = str.into_bytes();

                    let init_msg = [&meta_cp.state_code[0..2], &meta_cp.facility_id.to_ne_bytes(), &json.len().to_ne_bytes()].concat();

                    let _ = stream.write_all(&init_msg);
                    let mut buf: [u8; 8] = [0; 8];
                    let _ = stream.read(&mut buf).unwrap();
                    let _ = stream.write_all(&json);
                });
                println!("Scheduled Facility {} ({}-{:X}) to be scanned every {} minutes, starting at {}", CStr::from_bytes_until_nul(&meta.facility_name).unwrap().to_str().unwrap(), CStr::from_bytes_until_nul(&meta.state_code).unwrap().to_str().unwrap(), meta.facility_id, meta.run_interval, t);
            }
        }
    }

    loop {}
    Ok(())
}
