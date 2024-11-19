use std::string;
use std::collections::VecDeque;
use super::types;

pub mod preprocessor {
    pub struct Preprocessor {
        pub input: VecDeque<types::RawInmate>
    }
}
