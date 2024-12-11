mod id;
mod formatting;

pub mod processor {
    use crate::database::database::Connection;
    use crate::stream::stream::Stream;
    use crate::types::types::{RawInmate, Person};
    use super::id::id;
    use super::formatting::formatting;
    use std::sync::Arc;
    use std::boxed::Box;
    use std::thread;
    use std::sync::atomic::{AtomicBool, Ordering};
    use chrono::{Utc, Datelike};
    use chrono::prelude::*;

    const MAX_THREADS: u8 = 4;

    pub struct Processor {
        conn: Arc<Connection>,                
        input: Arc<Stream<RawInmate>>,
        threads: Vec<std::thread::JoinHandle<()>>,
        run: Arc<AtomicBool>,
    }

    impl Processor {
        pub fn new(input_stream: Arc<Stream<RawInmate>>, conn: Arc<Connection>) -> Box<Self> {
            return Box::new(Self {
                conn: Arc::clone(&conn),
                input: input_stream.clone(),
                threads: Vec::new(),
                run: Arc::new(AtomicBool::new(true)),
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

        fn process_inmate(mut inmate: RawInmate, conn: &Arc<Connection>) {
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
                    };

                    if !conn.insert_person(&person) {
                        // cry
                        todo!("cry");
                    }
                }




                
            }
        }
    }

    impl Drop for Processor {
        fn drop(&mut self) {
            self.despool();
        }
    }
}
