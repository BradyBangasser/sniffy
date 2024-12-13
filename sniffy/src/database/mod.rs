pub mod database {
    use mysql::*;
    use std::sync::{Mutex, Arc};
    use std::collections::HashMap;
    use std::thread;
    use crate::types::types::{Arrest, Person, RosterEntry, Charge};

    pub struct Connection {
        pool: Option<Pool>,
        person_cache: Mutex<HashMap<[u8; 32], Person>>,
        arrest_cache: Mutex<HashMap<u64, Arrest>>,
        charge_cache: Mutex<HashMap<u64, Charge>>,
    }

    impl Connection {
        pub fn new() -> Result<Arc<Self>, Box<dyn std::error::Error>> {
            let mut s = Self {
                pool: None,
                person_cache: Mutex::new(HashMap::new()),
                arrest_cache: Mutex::new(HashMap::new()),
                charge_cache: Mutex::new(HashMap::new()),
            };

            if cfg!(not(debug_assertions)) {
                let url = "sniffy@localhost:3307";
                s.roster = Mutex::new(HashMap::new());
                todo!("add pool");
            } 

            Ok(Arc::new(s))
        }

        pub fn query_current_roster(self: &Arc<Self>) {
            // todo!("Fetch roster");
            let mut roster: Vec<RosterEntry> = Vec::new();

            // lets overengineer the shit outta this
            let roster_thread = thread::spawn(|| {
                for entry in roster {
                    self.roster.lock().unwrap().insert(entry.aid, entry);
                }
            });

            roster_thread.join();

            return self.roster;
        }

        pub fn query_person_by_id(self: &Arc<Self>, id: &[u8; 32]) -> Option<()> {
            todo!("Database Query");
            None
        }

        pub fn insert_person(self: &Arc<Self>, person: &Person) -> bool {
            todo!("Insert person");
            false
        }

        pub fn count_arrests(self: &Arc<Self>) -> u64 {
            todo!("Add the actual db query");
            return 3;
        }
    }
}
