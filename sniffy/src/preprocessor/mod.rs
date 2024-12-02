use std::thread;

pub mod preprocessor {
    use crate::types::types::Inmate;
    use std::collections::VecDeque;

    const MAX_THREADS: u8 = 4;

    pub struct Preprocessor {
        pub output: VecDeque<Inmate>,
        threads: Vec<std::thread::JoinHandle<()>>,
    }

    impl Preprocessor {
        pub fn new() -> &mut Preprocessor {
            let mut pp = Preprocessor {
                output: VecDeque::<Inmate>::new(),
                threads: Vec::new() 
            };

            pp.spool();

            return &mut pp;
        }

        fn spool(&mut self) {
            for i in 0..MAX_THREADS {
                self.threads.index(0)
            }
        }
    }
}
