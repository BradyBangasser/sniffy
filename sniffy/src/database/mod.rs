pub mod database {
    use mysql::*;
    use std::sync::Arc;
    use crate::types::types::Person;

    pub struct Connection {
        pool: Option<Pool>,
    }

    impl Connection {
        pub fn new() -> Result<Arc<Self>, Box<dyn std::error::Error>> {
            if cfg!(not(debug_assertions)) {
                let url = "sniffy@localhost:3307";
                let s = Self {
                    pool: Some(Pool::new(url)?)
                };

                Ok(Arc::new(s))
            } else {
                let s = Self {
                    pool: None,
                };

                Ok(Arc::new(s))
            }
        }

        pub fn query_person_by_id(self: &Arc<Self>, id: &[u8; 32]) -> Option<()> {
            todo!("Database Query");
            None
        }

        pub fn insert_person(self: &Arc<Self>, person: &Person) -> bool {
            todo!("Insert person");
            false
        }
    }
}
