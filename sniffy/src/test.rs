pub mod test {
    use crate::types::types::{RawInmate, Person, RosterEntry};
    use crate::Stream;
    use chrono::TimeZone;
    use rand::prelude::*;
    use std::sync::Arc;
    /**
     * generated data, current roster, current database, last query
     */
    pub fn generate_test_data() -> (Vec<Person>, Vec<RosterEntry>, Vec<Person>, Arc<Stream<RawInmate>>) {
        let mut people = Vec::<Person>::new();
        let mut previous_roster = Vec::<RosterEntry>::new();
        let mut current_database = Vec::<Person>::new();
        let last_query = Stream::<RawInmate>::new();

        let mut range = rand::thread_rng();


        for i in 0..99 {
            let age = range.gen_range(14..169);
            let height = range.gen_range(4..100);
            let weight = range.gen_range(0..500);
            println!("{}", age);

            let person = Person {
                first_name: String::from("Testy"),
                last_name: String::from("McTesterface"),
                middle_name: i.to_string(),
                birth_year: chrono::Utc.with_ymd_and_hms(2024 - age, 1, 1, 1, 1, 1).unwrap(),
                height, 
                weight,
                added: chrono::Utc::now(),
                home_address: None,
                notes: String::new(),
                phone_number: None,
                sex: true,
                race: String::new(),
                updated: chrono::Utc::now(),
                versioning: String::new(),
                id: crate::processor::id::id::_compute_id(String::from("McTesterface"), String::from("Testy"), false, i.to_string(), 2024 - age).expect("Failed to generate ID"),
            };

            people.push(person.clone());

            match range.gen_range(0..3) {
                0 => {
                    previous_roster.push(RosterEntry { pid: person.id, aid: 0 });
                },
                1 => {
                    previous_roster.push(RosterEntry { pid: person.id, aid: 0 });
                    current_database.push(person);
                },
                2 => {
                    current_database.push(person.clone());
                    last_query.push(RawInmate {
                        first_name: String::from("Testy"),
                        middle_name: i.to_string(),
                        last_name: String::from("McTesterface"),
                        age: Some(age as u8),
                        sex: Some(false),
                        height: Some(height),
                        weight: Some(weight),
                        bond: Some(0),
                        birth_year: None,
                        arrest_date: Some(chrono::Utc::now()),
                        arresting_agency: Some(String::from("aa")),
                        booking_agency: None,
                        race: Some(String::from("w")),
                        arrest_notes: String::new(),
                        holding_facility: None,
                        release_date: None,
                        court_date: None,
                        image_id: None,
                        home_address: None,
                        case_id: None,
                        charges: Vec::new(),
                        notes: String::new()
                    });
                },
                3 => {
                    last_query.push(RawInmate {
                        first_name: String::from("Testy"),
                        middle_name: i.to_string(),
                        last_name: String::from("McTesterface"),
                        age: Some(age as u8),
                        sex: Some(false),
                        height: Some(height),
                        weight: Some(weight),
                        bond: Some(0),
                        birth_year: None,
                        arrest_date: Some(chrono::Utc::now()),
                        arresting_agency: Some(String::from("aa")),
                        booking_agency: None,
                        race: Some(String::from("w")),
                        arrest_notes: String::new(),
                        holding_facility: None,
                        release_date: None,
                        court_date: None,
                        image_id: None,
                        home_address: None,
                        case_id: None,
                        charges: Vec::new(),
                        notes: String::new()
                    });
                }

                _ => panic!("This shouldn't happen"),
            }
        }

        (people, previous_roster, current_database, last_query)
    }
}
