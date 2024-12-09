pub mod stream {
    pub struct Stream<T> {
        stream: Vec<T>,
    }

    impl<T> Stream<T> {
        pub fn new() -> Arc<Mutex<Vec<T>>> {

        }
    }
}
