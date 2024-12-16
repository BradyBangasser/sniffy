pub mod parsers;

pub mod preprocessor {
    use std::thread;
    use crate::stream::stream::Stream;
    use std::sync::{Mutex, Arc};
    use std::sync::atomic::{AtomicBool,AtomicU32,Ordering};
    use crate::types::types::{RawInmate, RawCharge};
    use crate::preprocessor::parsers::parsers;

    const MAX_THREADS: u8 = 1;

    pub struct Preprocessor {
        output: Arc<Stream<RawInmate>>,
        threads: Vec<std::thread::JoinHandle<()>>,
        input: Arc<Mutex<Vec<String>>>,
        n_input: Arc<AtomicU32>,
        run: Arc<AtomicBool>
    }

    impl Preprocessor {
        pub fn new(out_stream: Arc<Stream<RawInmate>>) -> Preprocessor {
            let mut pp = Preprocessor {
                output: Arc::clone(&out_stream),
                threads: Vec::new(),
                input: Arc::new(Mutex::new(Vec::new())),
                n_input: Arc::new(AtomicU32::new(0)),
                run: Arc::new(AtomicBool::new(true))
            };

            pp.spool();
            return pp;
        }

        fn process_field(inmate: &mut RawInmate, field: &str, value: &serde_json::Value) {
            match field {
                "firstname" => inmate.first_name = value.to_string(),
                "lastname" => inmate.last_name = value.to_string(),
                "middlename" => inmate.middle_name = value.to_string(),
                "age" => inmate.age = parsers::parse_serde_age(value),
                "sex" => inmate.sex = parsers::parse_serde_sex(value),
                "birthyear" => inmate.birth_year = parsers::parse_serde_date(value),
                "height" => inmate.height = parsers::parse_serde_height(value),
                "weight" => inmate.weight = parsers::parse_serde_weight(value),
                "totalbondamount" => inmate.bond = value.as_u64(),
                "arrestdate" => inmate.arrest_date = parsers::parse_serde_date(value),
                "arrestingagency" => inmate.arresting_agency = parsers::parse_serde_not_empty_string(value),
                "bookingagency" => inmate.booking_agency = parsers::parse_serde_not_empty_string(value),
                "race" => inmate.race = parsers::parse_serde_not_empty_string(value),
                "arrestnotes" => inmate.arrest_notes.push_str(&value.to_string()),
                "holdingfacility" => inmate.holding_facility = parsers::parse_serde_not_empty_string(value),
                "releasedate" => inmate.release_date = parsers::parse_serde_date(value),
                "courtdate" => inmate.court_date = parsers::parse_serde_date(value),
                "imageid" => inmate.image_id = parsers::parse_serde_not_empty_string(value),
                "homeaddress" => inmate.home_address = parsers::parse_serde_not_empty_string(value),
                "caseid"|"docketnumber" => inmate.case_id = parsers::parse_serde_not_empty_string(value),
                "charges" => RawCharge::parse_serde_charges(&mut inmate.charges, value),
                _ => {
                    let v = parsers::parse_serde_not_empty_as_string(value);

                    if v.is_some() {
                        inmate.notes.push_str(field);
                        inmate.notes.push_str(": ");
                        inmate.notes.push_str(&v.unwrap());
                        inmate.notes.push('\n');
                    }
                }
            }
        }

        fn parse_json(output: &Arc<Stream<RawInmate>>, json: String) {
            let mjson = json.to_lowercase();
            let v: serde_json::Value = serde_json::from_str(&mjson).unwrap();

            if v["inmates"].is_array() {
                for inmate in v["inmates"].as_array().unwrap() {
                    if inmate.is_object() {
                        let inmate_obj = inmate.as_object().unwrap();
                        let mut raw_inmate = RawInmate::new();
                        for val in inmate_obj.keys() {
                            Self::process_field(&mut raw_inmate, val, &inmate_obj[val]);
                        }

                        output.push(raw_inmate);
                    }
                }
            }
        }

        fn spool(self: &mut Self) {
            for _ in 0..MAX_THREADS {
                let input = Arc::clone(&self.input);
                let run = Arc::clone(&self.run);
                let n_input = Arc::clone(&self.n_input);
                let output = Arc::clone(&self.output);

                self.threads.push(
                    thread::spawn(move || {
                        loop {
                            let input_len = n_input.load(Ordering::Relaxed);

                            if input_len == 0 {
                                if !run.load(Ordering::Relaxed) {
                                    break;
                                }
                                thread::sleep(std::time::Duration::from_secs(1));

                                continue;
                            }

                            let mut invec = input.lock().unwrap();
                            match invec.pop() {
                                Some(v) => {
                                    n_input.store(input_len - 1, Ordering::Relaxed);
                                    Self::parse_json(&output, v)
                                },
                                None => continue,
                            };
                        }
                    })
                );
            }
        }

        pub fn push(self: &mut Self, str: String) {
            self.input.lock().unwrap().push(str);
            self.n_input.store(self.n_input.load(Ordering::Relaxed) + 1, Ordering::Relaxed);
        }

        pub fn despool(self: &mut Self) {
            println!("Despooling");
            self.run.store(false, Ordering::Relaxed);

            while self.threads.len() > 0 {
                let th = self.threads.remove(0);
                th.join().unwrap();
            }
        }
    }

    impl Drop for Preprocessor {
        fn drop(&mut self) {
            self.despool();
        }
    }
}
