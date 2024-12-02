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
        pub fn new() -> Preprocessor {
            let mut pp = Preprocessor {
                output: VecDeque::<Inmate>::new(),
                threads: Vec::new() 
            };

            pp.spool();

            return pp;
        }

        fn 

        fn spool(&mut self) {
            for i in 0..MAX_THREADS {
                thread::spawn(|| {})
            }
        }

        fn despool(&mut self) {
            
        }
    }
}
