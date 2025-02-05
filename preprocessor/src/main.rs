pub mod module_loader;
mod scheduler;
mod database;
mod fac_id;
mod roster_logger;

use std::sync::atomic::{AtomicUsize, Ordering};
use std::{collections::HashMap, os::unix::net::UnixStream};
use std::io::prelude::*;
use mysql::prelude::*;
use roster_logger::RosterLogger;
use std::ffi::{CStr, CString};
use self::scheduler::scheduler::Scheduler;
use chrono::{DateTime, Duration, Local, Timelike};
use std::sync::{Mutex, Arc};
use std::thread::sleep;
use serde::{Serialize, Deserialize};

#[derive(Serialize, Deserialize)]
struct People {
    size: u32,
    inmates: Vec<serde_json::Value>,
}

fn schedule_module_fetch(m_id: u16, meta: Arc<module_loader::ModuleMeta>, scheduler: Arc<Mutex<Scheduler>>) {
    let logger = Arc::new(Mutex::new(RosterLogger::new(&std::format!("{}-{:X}", String::from_utf8(meta.state_code[0..2].to_vec()).unwrap(), meta.facility_id))));
    let meta_c = meta.clone();

    let mut t: DateTime<Local> = Local::now();
    let mins: u8 = t.minute().try_into().unwrap();
    if  mins >= meta.start_time {
        t = t + Duration::hours(1);
        t = t - Duration::minutes((mins - meta.start_time).into());
    } else {
        t = t + Duration::minutes((meta.start_time - mins).into());
    }

    if meta.is_incremental != 0 {

        // let init_msg = [&meta.state_code[0..2], &meta.facility_id.to_ne_bytes(), &5_u32.to_ne_bytes(), &[meta.flags]].concat();

        let sch_ref = scheduler.clone();
        scheduler.lock().unwrap().push_interval_job(t, meta_c.run_interval, move || {
            println!("Executing module {} Fac ID: {}-{:08X}", CStr::from_bytes_until_nul(&meta_c.facility_name).unwrap().to_str().unwrap(), CStr::from_bytes_until_nul(&meta_c.state_code).unwrap().to_str().unwrap(), meta_c.facility_id);
            let results: Arc<Mutex<People>> = Arc::new(Mutex::new(People { size: 0, inmates: vec!() }));
            let cache: Arc<Mutex<HashMap<String, module_loader::ModuleOut>>> = Arc::new(Mutex::new(HashMap::new()));

            let mut initial_out = module_loader::ModuleOut {
                enc: 0,
                out: std::ptr::null(),
                len: 0,
            };

            unsafe { module_loader::exec_module(m_id, &mut initial_out); }

            let json = unsafe { serde_json::from_str(CStr::from_ptr(initial_out.out).to_str().unwrap()) };
            if json.is_err() {
                return;
            }

            let obj = json.unwrap();

            if let serde_json::Value::Object(elements) = obj {
                let mut t = Local::now();
                let inc = Duration::seconds(1);

                let keys = elements.keys();

                let t_count = Arc::new(AtomicUsize::new(keys.len() - 1));
                for key in keys {
                    let log_ref = logger.clone();
                    let res_ref = results.clone();
                    let meta_ref = meta_c.clone();
                    let t_count_ref = t_count.clone();

                    if cache.lock().unwrap().get(key).is_some() {
                        continue;
                    }

                    let s = Box::new(key.clone());
                    sch_ref.lock().unwrap().push_job(t, move || {
                        let mut out = module_loader::ModuleOut {
                            enc: 0,
                            out: std::ptr::null(),
                            len: 0,
                        };

                        let cstr = CString::new(s.as_str()).unwrap();
                        let res = unsafe { module_loader::exec_module_inc(cstr.as_ptr(), m_id, &mut out) };

                        let res_str = unsafe { CStr::from_ptr(out.out).to_str().unwrap().to_string() };

                        if res != 0 {
                            let mut err = HashMap::<String, String>::new();
                            err.insert(String::from("id"), *s.clone());
                            err.insert(String::from("res"), res_str);
                            log_ref.lock().unwrap().add_error(err);
                        } else {
                            res_ref.lock().unwrap().inmates.push(serde_json::from_str(&res_str.to_lowercase()).unwrap());
                        }

                        let threads = t_count_ref.fetch_sub(1, Ordering::SeqCst);
                        
                        if  threads == 0 {
                            let mut r = res_ref.lock().unwrap();
                            r.size = r.inmates.len().try_into().unwrap();
                            let json = serde_json::to_string(&*r).unwrap().into_bytes();

                            let mut stream = UnixStream::connect("/tmp/sniffy.socket").unwrap();
                            stream.set_nonblocking(false).expect("Failed to set socket to blocking");

                            let init_msg = [&meta_ref.state_code[0..2], &meta_ref.facility_id.to_ne_bytes(), &(TryInto::<u32>::try_into(json.len()).unwrap()).to_ne_bytes(), &[meta_ref.flags]].concat();

                            let _ = stream.write_all(&init_msg);
                            let mut buf: [u8; 8] = [0; 8];
                            let _ = stream.read(&mut buf).unwrap();
                            let _ = stream.write_all(&json);
                        } else {
                            sleep(std::time::Duration::from_secs(1));
                        }
                    });

                    t = t + inc;
                }

            }
        });
        println!("Scheduled Facility {} ({}-{:X}) to be scanned every {} minutes, starting at {}", CStr::from_bytes_until_nul(&meta.facility_name).unwrap().to_str().unwrap(), CStr::from_bytes_until_nul(&meta.state_code).unwrap().to_str().unwrap(), meta.facility_id, meta.run_interval, t);
    } else {
        println!("HERE");
        scheduler.lock().unwrap().push_interval_job(t, meta.run_interval, move || {
            let mut stream = UnixStream::connect("/tmp/sniffy.socket").unwrap();
            stream.set_nonblocking(false).expect("Failed to set socket to blocking");
            let mut out = module_loader::ModuleOut {
                enc: 0,
                out: std::ptr::null(),
                len: 0,
            };
            println!("Executing module {} Fac ID: {}-{:08X}", CStr::from_bytes_until_nul(&meta_c.facility_name).unwrap().to_str().unwrap(), CStr::from_bytes_until_nul(&meta_c.state_code).unwrap().to_str().unwrap(), meta_c.facility_id);
            unsafe { module_loader::exec_module(m_id, &mut out); }

            let mut str = unsafe { String::from(std::ffi::CStr::from_ptr(out.out).to_str().unwrap()) };
            str = str.to_lowercase();
            let json = str.into_bytes();

            let init_msg = [&meta_c.state_code[0..2], &meta_c.facility_id.to_ne_bytes(), &json.len().to_ne_bytes(), &[meta_c.flags]].concat();

            let _ = stream.write_all(&init_msg);
            let mut buf: [u8; 8] = [0; 8];
            let _ = stream.read(&mut buf).unwrap();
            let _ = stream.write_all(&json);
        });
        println!("Scheduled Facility {} ({}-{:X}) to be scanned every {} minutes, starting at {}", CStr::from_bytes_until_nul(&meta.facility_name).unwrap().to_str().unwrap(), CStr::from_bytes_until_nul(&meta.state_code).unwrap().to_str().unwrap(), meta.facility_id, meta.run_interval, t);
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let dberr = database::database::connect();
    let scheduler = Arc::new(Mutex::new(Scheduler::new()));

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

            schedule_module_fetch(module.id, Arc::new(meta), scheduler.clone());
        }

    }

    loop {}
}
