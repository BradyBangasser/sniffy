
pub mod preprocessor {
    use crate::types::types::Inmate;
    use std::collections::VecDeque;

    pub struct Preprocessor {
        pub output: VecDeque<Inmate>;
        threads
    }

    impl Preprocessor {
        pub fn new() -> Preprocessor {
            return Preprocessor {
                output: VecDeque::<Inmate>::new(),
            }
        }

        fn spool() {

        }
    }
}
