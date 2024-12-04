mod parsers;

pub mod preprocessor {
    use std::thread;
    use std::sync::{Mutex, Arc};
    use std::sync::atomic::{AtomicBool,AtomicU32,Ordering};
    use crate::types::types::Inmate;
    use crate::preprocessor::parsers::parsers;
    use std::collections::VecDeque;

    const MAX_THREADS: u8 = 1;

    #[derive(Debug)]
    struct _RawCharge {
        bond: Option<u64>,
        bond_status: Option<String>,
        datetime: Option<chrono::DateTime<chrono::Utc>>,
        statute_id: Option<String>,
        statute: Option<String>,
        statute_description: Option<String>,
        docket_number: Option<String>,
        notes: String
    }

    impl _RawCharge {
        pub fn new() -> Self {
            Self {
                bond: None,
                bond_status: None,
                datetime: None,
                statute_id: None,
                statute: None,
                statute_description: None,
                docket_number: None,
                notes: String::new()
            }
        }

        pub fn parse_serde_charges(v: &mut Vec<_RawCharge>, value: &serde_json::Value) {
            if !value.is_array() {
                return;
            }

            let charges = value.as_array();

            for charge in charges.unwrap() {
                let parsed_charge = Self::parse_serde(charge);

                if parsed_charge.is_some() {
                    v.push(parsed_charge.unwrap());
                }
            }
        }

        pub fn parse_serde(value: &serde_json::Value) -> Option<Self> {
            if !value.is_object() {
                return None;
            }

            let obj = value.as_object().unwrap();
            let mut charge = Self::new();

            for key in obj.keys() {
                match key.as_str() {
                    "bond"|"bondamount" => charge.bond = obj[key].as_u64(),
                    "bondstatus" => charge.bond_status = Some(obj[key].to_string()),
                    "date" => charge.datetime = parsers::parse_serde_date(&obj[key]),
                    "name" => charge.statute_id = parsers::parse_serde_not_empty_string(&obj[key]),
                    "description" => charge.statute = parsers::parse_serde_not_empty_string(&obj[key]),
                    "docketnumber" => charge.docket_number = parsers::parse_serde_not_empty_string(&obj[key]),
                    _ => {

                    }
                };
            }

            return Some(charge);
        }
    }

    #[derive(Debug)]
    struct _RawInmate {
        first_name: String,
        middle_name: String,
        last_name: String,
        age: Option<u64>,
        sex: Option<bool>,
        height: Option<u8>,
        weight: Option<u8>,
        bond: Option<u64>,
        birth_year: Option<chrono::DateTime<chrono::Utc>>,
        arrest_date: Option<chrono::DateTime<chrono::Utc>>,
        arresting_agency: Option<String>,
        booking_agency: Option<String>,
        race: Option<String>,
        arrest_notes: String,
        holding_facility: Option<String>,
        release_date: Option<chrono::DateTime<chrono::Utc>>,
        court_date: Option<chrono::DateTime<chrono::Utc>>,
        image_id: Option<String>,
        home_address: Option<String>,
        case_id: Option<String>,
        charges: Vec<_RawCharge>,
        notes: String
    }

    impl _RawInmate {
        pub fn new() -> Self {
            return _RawInmate {
                first_name: String::new(),
                last_name: String::new(),
                middle_name: String::new(),
                age: None,
                sex: None,
                height: None,
                weight: None,
                bond: None,
                birth_year: None,
                arrest_date: None,
                arresting_agency: None,
                booking_agency: None,
                race: None,
                arrest_notes: String::new(),
                holding_facility: None,
                release_date: None,
                court_date: None,
                image_id: None,
                home_address: None,
                case_id: None,
                charges: Vec::new(),
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
                "age" => inmate.age = value.as_u64(),
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
                "charges" => _RawCharge::parse_serde_charges(&mut inmate.charges, value),
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

        pub fn drop(&mut self) {

        }
    }
}
