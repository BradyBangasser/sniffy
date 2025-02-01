use chrono::Local;
use serde::Serialize;
use std::{collections::HashMap, io::Write};

#[derive(Serialize)]
pub struct RosterLogger {
    time: String,
    module: String,
    // This is extremely ineffient, fix later
    errors: Vec<HashMap<String, String>>,
}

impl RosterLogger {
    pub fn new(module: &str) -> RosterLogger {
        return RosterLogger {
            time: Local::now().to_rfc3339(),
            module: String::from(module),
            errors: Vec::new(),
        };
    }

    pub fn add_error(&mut self, mut error: HashMap<String, String>) {
        error.shrink_to_fit();
        self.errors.push(error);
    }
}

impl Drop for RosterLogger {
    fn drop(&mut self) {
        let res = serde_json::to_string(self).unwrap();
        let mut f = std::fs::File::create(std::format!("/tmp/spp_{}.json", Local::now().timestamp())).unwrap();
        let _ = f.write_all(res.as_bytes()); 
    }
}
