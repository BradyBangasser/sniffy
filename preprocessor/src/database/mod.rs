pub mod database {
    use std::error::Error;
    use std::sync::OnceLock;

    use mysql::*;

    const URL: &str = "mysql://sniffy@localhost:3306/sniffy";

    static POOL: OnceLock<Pool> = OnceLock::new();

    pub fn connect() -> Result<(), Box<dyn Error>> {
        POOL.set(Pool::new(URL)?).expect("Failed to initialize connection");
        return Ok(());
    }
    
    pub fn get_pool() -> Option<Pool> {
        let p = POOL.get();
        if p.is_some() {
            return Some(p.unwrap().clone());
        }

        return None;
    }
}
