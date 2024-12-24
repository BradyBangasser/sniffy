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
        tx: Option<Box<Sender<Option<Arc<Person>>>>>
    }

    pub struct Connection {
        pool: Option<Pool>,
        test_roster: Arc<Mutex<Vec<RosterEntry>>>, // Previous roster, current Database
        roster_cache: Option<Arc<Mutex<HashMap<[u8; 32], RosterEntry>>>>,
        test_database: Arc<Mutex<Vec<Person>>>,
        test_arrests: Arc<Mutex<Vec<Arrest>>>,
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
                roster_cache: None,
                test_roster: Arc::new(Mutex::new(Vec::new())),
                test_database: Arc::new(Mutex::new(Vec::new())),
                test_arrests: Arc::new(Mutex::new(Vec::new())),
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

        pub fn new_test(previous_roster: Vec<RosterEntry>, current_database: Vec<Person>, arrests: Vec<Arrest>) -> Result<Self, Box<dyn std::error::Error>> {
            let mut s = Self::new()?;
            s.test_roster = Arc::new(Mutex::new(previous_roster));
            s.test_database = Arc::new(Mutex::new(current_database));
            s.test_arrests = Arc::new(Mutex::new(arrests));
            return Ok(s);
        }

        fn spool(&mut self) {
            // Spool up the people queryers
            //         idk if that ^ is a word but I had no clue what word to use

            for _ in [0..PERSON_QUERY_THREADS] {
                let run = Arc::clone(&self.run);
                let pqq = Arc::clone(&self.person_query_queue);
                let pcache = Arc::clone(&self.person_cache);
                let test_database = Arc::clone(&self.test_database);

                self.person_query_threads.push(thread::spawn(move || {
                    loop {
                        let pqw = pqq.lock().unwrap().pop_front();

                        if pqw.is_none() {
                            if !run.load(Ordering::Relaxed) {
                                break;
                            }
                            continue;
                        }

                        let pq = pqw.unwrap();
                        
                        match pq.query {
                            // I hate rust
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
                                    continue;
                                } else {
                                    println!("Fetching id {:x?} from database", id);

                                    if cfg!(not(debug_assertions)) {
                                        todo!("prod");
                                    } else {
                                        let p = Self::check_test_db(Arc::clone(&test_database), &id);

                                        if pq.tx.is_some() {
                                            let _ = pq.tx.unwrap().send(p);
                                            println!("Found in database");
                                        } else {
                                            let _ = pq.tx.unwrap().send(None);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }));
            }
        }

        fn check_test_db(db: Arc<Mutex<Vec<Person>>>, id: &[u8; 32]) -> Option<Arc<Person>> {
            for p in db.lock().unwrap().iter() {
                if p.id.eq(id) {
                    return Some(Arc::new(p.clone()));
                }
            }

            None
        }

        pub fn query_current_roster(self: &Self) -> Arc<Mutex<HashMap<[u8; 32], RosterEntry>>> {
            // todo!("Fetch roster");
            let mut roster: Arc<Mutex<HashMap<[u8; 32], RosterEntry>>>;

            if self.roster_cache.is_some() {
                return self.roster_cache.clone().unwrap();
            }

            if cfg!(not(debug_assertions)) {
                roster = Arc::new(Mutex::new(HashMap::new()));
            } else {
                let mut ros: HashMap<[u8; 32], RosterEntry> = HashMap::new();
                for e in self.test_roster.lock().unwrap().iter() {
                    ros.insert(e.pid, e.clone());
                }

                roster = Arc::new(Mutex::new(ros));
            }

            // push data into map
            // STUFF HERE
            
            // Pre fetch those elements, to cache them


            return roster;
        }

        pub fn query_person_by_id(self: &Self, id: &[u8; 32]) -> Option<Arc<Person>> {
            let (tx, rx) = channel();            

            let query = PersonQueryRequest {
                query: PersonQueryKey::ID(id.clone()),
                tx: Some(Box::new(tx)),
            };

            self.person_query_queue.lock().unwrap().push_back(query);

            println!("{:?}", rx.recv());

            // return rx.recv_timeout(Duration::from_millis(250)).unwrap_or(None);
            return rx.recv().unwrap();
        }

        pub fn insert_person(self: &Self, person: Arc<Person>) -> bool {
            // todo!("Insert person");
            println!("insert {:x?}", person.id);
            true
        }

        pub fn count_arrests(self: &Self) -> u64 {
            // todo!("Add the actual db query");
            return 0;
        }

        pub fn query_arrest_by_id(self: &Self, aid: u64) -> Option<Arrest> {
            // TODO, make this threaded and cached

            for a in self.test_arrests.lock().unwrap().iter() {
                if a.id == aid {
                    return Some(a.clone());
                }
            }

            return None;
        }
    }
}
