pub mod processor {
    use crate::database::database::Connection;
    pub struct Processor {
        conn: Option<Connection>,                
        input: Arc<Mutex<Vec<RawInmate>>>,
    }

    impl Processor {
        pub fn new()
    }
}
