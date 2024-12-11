pub mod stream {
    use std::sync::{Arc, Mutex};
    use std::collections::VecDeque;

    #[derive(Debug)]
    pub struct Stream<T> {
        stream: Arc<Mutex<VecDeque<T>>>,
    }

    impl<T> Stream<T> {
        pub fn new() -> Arc<Self> {
            Arc::new(Stream {
                stream: Arc::new(Mutex::new(VecDeque::new()))
            })
        }

        pub fn pop(self: &Arc<Self>) -> Option<T> {
            return self.stream.lock().unwrap().pop_front();
        }

        pub fn push(self: &Arc<Self>,  element: T) {
            self.stream.lock().unwrap().push_back(element);
        }

        pub fn is_empty(self: &Arc<Self>) -> bool {
            self.stream.lock().unwrap().is_empty()
        }
    }
}
