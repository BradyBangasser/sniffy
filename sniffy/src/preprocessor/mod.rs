pub mod preprocessor {
    use std::thread;
    use std::sync::{Mutex, Arc};
    use std::sync::atomic::{AtomicBool,AtomicU32,Ordering};
    use crate::types::types::Inmate;
    use std::collections::VecDeque;

    const MAX_THREADS: u8 = 1;

    #[derive(Debug)]
    struct _RawInmate {
        first_name: String,
        middle_name: String,
        last_name: String,
        age: Option<u8>,
        birth_year: Option<chrono::DateTime<chrono::Utc>>,
        notes: String
    }

    impl _RawInmate {
        pub fn new() -> Self {
            return _RawInmate {
                first_name: String::new(),
                last_name: String::new(),
                middle_name: String::new(),
                age: None,
                birth_year: None,
                notes: String::new()
            }
        }
    }

    #[derive(Debug)]
    pub struct Preprocessor {
        pub output: Arc<Mutex<VecDeque<Inmate>>>,
        threads: Vec<std::thread::JoinHandle<()>>,
        input: Arc<Mutex<Vec<String>>>,
        n_input: Arc<AtomicU32>,
        run: Arc<AtomicBool>
    }

    impl Preprocessor {
        pub fn new() -> Preprocessor {
            let mut pp = Preprocessor {
                output: Arc::new(Mutex::new(VecDeque::<Inmate>::new())),
                threads: Vec::new(),
                input: Arc::new(Mutex::new(Vec::new())),
                n_input: Arc::new(AtomicU32::new(0)),
                run: Arc::new(AtomicBool::new(true))
            };

            pp.spool();
            return pp;
        }

        fn process_field(inmate: &mut _RawInmate, field: &str, value: &serde_json::Value) {
            match field {
                "firstname" => inmate.first_name = value.to_string(),
                "lastname" => inmate.last_name = value.to_string(),
                "middlename" => inmate.middle_name = value.to_string(),
                _ => {
                    inmate.notes.push_str(field);
                    inmate.notes.push_str(": ");
                    inmate.notes.push_str(&value.to_string());
                    inmate.notes.push_str("\n");
                }
            }
        }

        fn parse_json(json: String) -> Option<Vec<_RawInmate>> {
            let mjson = json.to_lowercase();
            let v: serde_json::Value = serde_json::from_str(&mjson).unwrap();
            let mut inmates: Vec<_RawInmate> = Vec::new();

            if v["inmates"].is_array() {
                println!("len: {}", v["inmates"].as_array().unwrap().len());
                for inmate in v["inmates"].as_array().unwrap() {
                    if inmate.is_object() {
                        let inmate_obj = inmate.as_object().unwrap();
                        let mut raw_inmate = _RawInmate::new();
                        for val in inmate_obj.keys() {
                            Self::process_field(&mut raw_inmate, val, &inmate_obj[val]);
                        }
                        inmates.push(raw_inmate);
                    } else {
                        return None;
                    }
                }

                Some(inmates)
            } else {
                None
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
                                println!("{}", run.load(Ordering::Relaxed));

                                continue;
                            }

                            let mut invec = input.lock().unwrap();
                            let json = match invec.pop() {
                                Some(v) => {
                                    n_input.store(input_len - 1, Ordering::Relaxed);
                                    Self::parse_json(v)
                                },
                                None => continue,
                            };

                            for i in json.expect("AHHH") {
                                println!("{}", i.notes);
                            }
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

        fn drop(&mut self) {

        }
    }
}
