pub mod types {
    pub struct RawInmate<'a> {
        pub encoding: &'a str,
        pub data: &'a str,
    }

    #[derive(Debug)]
    pub struct Inmate {
        pub first: String,
        pub middle: String,
        pub last: String,
        pub birth_year: i16,
    }
}
