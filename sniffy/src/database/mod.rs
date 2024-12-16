pub mod database {
    use mysql::*;
    use std::sync::{Mutex, Arc};
    use std::collections::{HashMap, VecDeque};
    use std::sync::mpsc::{channel, Sender};
    use std::sync::atomic::{AtomicBool, Ordering};
    use std::thread;
    use std::time::Duration;
    use crate::types::types::{Arrest, Person, RosterEntry, Charge};

    const PERSON_QUERY_THREADS: u8 = 1;

    enum PersonQueryKey {
        NAME{first: String, middle: Option<String>, last: String},
        ID([u8; 32]),
    }

    struct PersonQueryRequest {
        query: PersonQueryKey,
        tx: Option<Sender<Option<Arc<Person>>>>
    }

    pub struct Connection {
        pool: Option<Pool>,
        test_data: Option<(Vec<RosterEntry>, Vec<Person>)>, // Previous roster, current Database
        person_cache: Arc<Mutex<HashMap<[u8; 32], Arc<Person>>>>,
        person_query_threads: Vec<thread::JoinHandle<()>>,
        person_query_queue: Arc<Mutex<VecDeque<PersonQueryRequest>>>,
        run: Arc<AtomicBool>,
        arrest_cache: Mutex<HashMap<u64, Arrest>>,
        charge_cache: Mutex<HashMap<u64, Charge>>,
    }

    impl Connection {
        pub fn new() -> Result<Self, Box<dyn std::error::Error>> {
            let mut s = Self {
                pool: None,
                test_data: None,
                person_cache: Arc::new(Mutex::new(HashMap::new())),
                person_query_threads: Vec::new(),
                person_query_queue: Arc::new(Mutex::new(VecDeque::new())),
                run: Arc::new(AtomicBool::new(true)),
                arrest_cache: Mutex::new(HashMap::new()),
                charge_cache: Mutex::new(HashMap::new()),
            };

            if cfg!(not(debug_assertions)) {
                let url = "sniffy@localhost:3307";
                s.arrest_cache = Mutex::new(HashMap::new());
                todo!("add pool");
            } 

            s.spool();
            Ok(s)
        }

        pub fn new_test(previous_roster: Vec<RosterEntry>, current_database: Vec<Person>) -> Result<Self, Box<dyn std::error::Error>> {
            let mut s = Self::new()?;
            s.test_data = Some((previous_roster, current_database));
            return Ok(s);
        }

        fn spool(&mut self) {
            // Spool up the people queryers
            //         idk if that ^ is a word but I had no clue what word to use

            for _ in [0..PERSON_QUERY_THREADS] {
                let run = Arc::clone(&self.run);
                let pqq = Arc::clone(&self.person_query_queue);
                let pcache = Arc::clone(&self.person_cache);

                self.person_query_threads.push(thread::spawn(move || {
                    loop {
                        let pqw = pqq.lock().unwrap().pop_front();

                        if pqw.is_none() {
                            if !run.load(Ordering::Relaxed) {
                                break;
                            }

                            thread::sleep(Duration::from_secs(1));
                        }

                        let pq = pqw.unwrap();
                        
                        match pq.query {
                            PersonQueryKey::NAME{first, middle, last} => {
                                todo!("database");
                            },
                            PersonQueryKey::ID(id) => {
                                let cache = pcache.lock().unwrap();
                                let cache_hit = cache.get(&id);

                                if cache_hit.is_some() {
                                    println!("Cache hit on id {:x?}", id);
                                    if pq.tx.is_some() {
                                        let _ = pq.tx.unwrap().send(Some(cache_hit.unwrap().clone()));
                                    }
                                } else {
                                    println!("Fetching id {:x?} from database", id);
                                    todo!("database");
                                }
                            }
                        }
                    }
                }));
            }

        }

        pub fn query_current_roster(self: &Self) -> HashMap<[u8; 32], RosterEntry> {
            // todo!("Fetch roster");
            let mut roster: HashMap<[u8; 32], RosterEntry> = HashMap::new();

            // push data into map
            // STUFF HERE
            
            // Pre fetch those elements, to cache them


            return roster;
        }

        pub fn query_person_by_id(self: &Self, id: &[u8; 32]) -> Option<Arc<Person>> {
            let (tx, rx) = channel();            

            let query = PersonQueryRequest {
                query: PersonQueryKey::ID(id.clone()),
                tx: Some(tx),
            };

            self.person_query_queue.lock().unwrap().push_back(query);

            return rx.recv_timeout(Duration::from_millis(250)).unwrap_or(None);
        }

        pub fn insert_person(self: &Self, person: &Person) -> bool {
            todo!("Insert person");
            false
        }

        pub fn count_arrests(self: &Self) -> u64 {
            // todo!("Add the actual db query");
            return 0;
        }
    }
}
