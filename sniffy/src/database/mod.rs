pub mod database {
    use mysql::*;

    pub struct Connection {
        pool: Option<Pool>,
    }

    impl Connection {
        pub fn new() -> Result<Self, Box<dyn std::error::Error>> {
            if cfg!(not(debug_assertions)) {
                let url = "sniffy@localhost:3307";
                let s = Self {
                    pool: Some(Pool::new(url)?)
                };

                Ok(s)
            } else {
                let s = Self {
                    pool: None,
                };

                Ok(s)
            }
        }

        
    }
}
