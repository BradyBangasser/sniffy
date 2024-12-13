mod id;
mod formatting;

pub mod processor {
    use crate::database::database::Connection;
    use crate::stream::stream::Stream;
    use crate::types::types::{RawInmate, Person, Arrest, Charge, RawCharge};
    use super::id::id;
    use super::formatting::formatting;
    use std::sync::Arc;
    use std::boxed::Box;
    use std::thread;
    use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
    use chrono::{Utc, Datelike};
    use chrono::prelude::*;

    const MAX_THREADS: u8 = 4;

    pub struct Processor {
        conn: Arc<Connection>,                
        input: Arc<Stream<RawInmate>>,
        threads: Vec<std::thread::JoinHandle<()>>,
        run: Arc<AtomicBool>,
        aid: Arc<AtomicU64>,
    }

    impl Processor {
        pub fn new(input_stream: Arc<Stream<RawInmate>>, conn: Arc<Connection>) -> Box<Self> {
            return Box::new(Self {
                conn: Arc::clone(&conn),
                input: input_stream.clone(),
                threads: Vec::new(),
                run: Arc::new(AtomicBool::new(true)),
                aid: Arc::new(AtomicU64::new(conn.count_arrests())),
            })
        }

        fn spool(&mut self) {
            for _ in [0..MAX_THREADS] {
                let input = Arc::clone(&self.input);
                let run = Arc::clone(&self.run);
                let conn = Arc::clone(&self.conn);

                self.threads.push(
                    thread::spawn(move || {
                        loop {
                            if input.is_empty() {
                                if !run.load(Ordering::Relaxed) {
                                    break;
                                }

                                thread::sleep(std::time::Duration::from_secs(1));
                                continue;
                            }

                            let inmate = input.pop();

                            if inmate.is_none() {
                                continue;
                            }

                            Self::process_inmate(inmate.unwrap(), &conn);
                        }
                    })
                )
            }
        }

        fn despool(&mut self) {
            if self.threads.is_empty() {
                return;
            }

            self.run.swap(false, Ordering::Relaxed);

            while !self.threads.is_empty() {
                let _ = self.threads.pop().unwrap().join();
            }
        }

        fn process_inmate(mut inmate: RawInmate, conn: &Arc<Connection>, aidc: Arc<AtomicU64>) ->Option<()> {
            if inmate.birth_year.is_none() {
                if inmate.age.is_none() {
                    todo!("Handle null age");
                }

                let now = Utc::now();
                let age = inmate.age.unwrap();
                inmate.birth_year = Some(Utc.with_ymd_and_hms(now.year() - age as i32, 0, 0, 0, 0, 0).unwrap());

            }

            let person_id_wrapped = id::compute_id(&inmate);

            if person_id_wrapped.is_none() {
                todo!("Handler failure to create Inmate ID");
            }

            let person_id = person_id_wrapped.unwrap();

            let person_record = conn.query_person_by_id(&person_id);

            let mut person: Person;

            if person_record.is_some() {
                person = person_record.unwrap();
            }

            if todo!("Check current roster") {
                // Logic for if the user is currently on the inmate roster
                let arrest = todo!("Get relevent arrest");
            } else {
                if person_record.is_none() {
                    person = Person {
                        id: person_id,
                        first_name: formatting::capitalize_name(inmate.first_name),
                        last_name: formatting::capitalize_name(inmate.last_name),
                        middle_name: formatting::capitalize_name(inmate.middle_name),
                        sex: inmate.sex.unwrap(),
                        height: inmate.height.unwrap(),
                        weight: inmate.weight.unwrap(),
                        race: inmate.race.unwrap(),
                        home_address: inmate.home_address,
                        added: chrono::Utc::now(),
                        birth_year: inmate.birth_year.expect("Birth year cannot be null"),
                        phone_number: None,
                        notes: String::new(),
                        updated: Utc::now(),
                        versioning: String::new(),
                    };

                    if !conn.insert_person(&person) {
                        // cry
                        todo!("cry");
                    }
                }

                if inmate.arresting_agency.is_none() {
                    if inmate.booking_agency.is_some() {
                        inmate.arresting_agency = inmate.booking_agency.clone();
                    } else {
                        todo!("Module default agency");
                    }
                }

                let aid = aidc.fetch_add(1, Ordering::Relaxed);

                let (booked_at, charges) = Self::parse_charges(aid, inmate.charges)?;

                // Create arrest
                let mut arrest = Arrest {
                    agency_id: id::compute_agency_id(&inmate.arresting_agency.unwrap()),
                    bond: inmate.bond.unwrap_or(0),
                    initial_bond: inmate.bond.unwrap_or(0),
                    holding_facility_id: id::compute_facility_id(&inmate.holding_facility.unwrap()),
                    id: aid,
                    pid: person.id,
                    notes: inmate.notes,
                    versioning: String::new(),
                    release_date: inmate.release_date,
                    booked: booked_at,
                };

                todo!("File arrest and charges");
            }

            None
        }

        pub fn parse_charges(aid: u64, raw_charges: Vec<RawCharge>) -> Option<(DateTime<Utc>, Vec<Charge>)> {
            let mut charges = Vec::<Charge>::new();
            let mut booked_at: DateTime<Utc> = Utc::now(); // eariest charged_at time
            for raw_charge in raw_charges {

                if raw_charge.statute_id.is_none() {
                    return None;
                }

                let mut sid = String::from("IA-");
                sid.push_str(&raw_charge.statute_id.unwrap());

                if raw_charge.datetime.is_some() {
                    if booked_at > raw_charge.datetime? {
                        booked_at = raw_charge.datetime.unwrap();
                    }
                }

                charges.push(Charge {
                    id: 0, // Autoinc in database, idc about the value until after insertion
                    bond: raw_charge.bond.unwrap_or(0),
                    initial_bond: raw_charge.bond.unwrap_or(0),
                    bond_status: raw_charge.bond_status.unwrap_or(String::from("active")), // Just assume the
                    sid,
                    aid,
                    docket_number: raw_charge.docket_number,
                    court_date: None,
                    timestamp: raw_charge.datetime.unwrap_or(booked_at),
                    versioning: String::new(),
                    notes: raw_charge.notes,
                })
            }
            None
        }
    }

    impl Drop for Processor {
        fn drop(&mut self) {
            self.despool();
        }
    }
}
