pub mod scheduler {
    use chrono::{DateTime, Local};
    use std::cmp::Reverse;
    use std::thread::{self, sleep, JoinHandle};
    use std::collections::BinaryHeap;
    use std::sync::{Arc, Mutex};
    use std::time;

    struct Job {
        time: DateTime<Local>,
        _interval: Option<u8>,
        closure: Box<dyn Fn() -> () + Send + 'static>,
    }

    impl PartialEq for Job {
        fn eq(&self, other: &Self) -> bool {
            return self.time.eq(&other.time);
        }
    }

    impl Eq for Job {}

    impl PartialOrd for Job {
        fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
            return self.time.partial_cmp(&other.time);
        }
    }

    impl Ord for Job {
        fn cmp(&self, other: &Self) -> std::cmp::Ordering {
            return self.time.cmp(&other.time);
        }
    }

    pub struct Scheduler {
        job_queue: Arc<Mutex<BinaryHeap<Reverse<Box<Job>>>>>,
        _worker: JoinHandle<()>,
    }

    impl Scheduler {
        fn run_loop(job_queue: Arc<Mutex<BinaryHeap<Reverse<Box<Job>>>>>) {
            loop {
                let mut mjq = job_queue.lock().unwrap();
                if !mjq.is_empty() {
                    let now = Local::now();
                    while let Some(job) = mjq.peek() {
                        if job.0.time > now { break }
                        let j = mjq.pop().unwrap();
                        thread::spawn(move || {
                            (j.0.closure)();
                        });
                    }
                }

                drop(mjq);
                sleep(time::Duration::from_millis(500));
            }
        }

        pub fn new() -> Scheduler {
            let jq = Arc::new(Mutex::new(BinaryHeap::new()));
            let jqc = Arc::clone(&jq);
            let s = Scheduler {
                job_queue: jq,
                _worker: thread::spawn(move || {
                    Scheduler::run_loop(jqc);
                })
            };

            return s;
        }

        pub fn push_job<T>(&mut self, t: DateTime<Local>, closure: T)
            where 
                T: Fn() -> () + Send + 'static
            {
                self.job_queue.lock().unwrap().push(Reverse(Box::new(Job {
                    time: t,
                    _interval: None,
                    closure: Box::new(closure)
                })));
            }

        pub fn push_interval_job<T>(&mut self, t: DateTime<Local>, interval: u8, closure: T)
            where 
                T: Fn() -> () + Send + 'static
        {
                self.job_queue.lock().unwrap().push(Reverse(Box::new(Job {
                    time: t,
                    _interval: Some(interval),
                    closure: Box::new(closure)
                })));
        }
    }

    impl Drop for Scheduler {
        fn drop(&mut self) {
        }
    }
}
