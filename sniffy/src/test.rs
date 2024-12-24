pub mod test {
    use crate::types::types::{RawInmate, Person, RosterEntry, Arrest};
    use crate::Stream;
    use chrono::{Timelike, TimeZone, Datelike};
    use rand::prelude::*;
    use std::sync::Arc;
    /**
     * generated data, current roster, current database, last query
     */
    pub fn generate_test_data() -> (Vec<Person>, Vec<RosterEntry>, Vec<Person>, Arc<Stream<RawInmate>>, Vec<Arrest>) {
        let mut people = Vec::<Person>::new();
        let mut previous_roster = Vec::<RosterEntry>::new();
        let mut current_database = Vec::<Person>::new();
        let last_query = Stream::<RawInmate>::new();
        let mut arrests = Vec::<Arrest>::new();

        let mut range = rand::thread_rng();
        let mut aid_num = 0;

        for i in 0..99 {
            let age = range.gen_range(14..169);
            let height = range.gen_range(4..100);
            let weight = range.gen_range(0..500);

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

            let book_date = chrono::Utc::now();
            let mut arrest_date = book_date;

            if range.gen_bool(0.5) {
                arrest_date = chrono::Utc.with_ymd_and_hms(book_date.year(), book_date.month(), book_date.month() - 1, book_date.hour(), book_date.minute(), book_date.second()).unwrap();
            }

            let initial_bond = if range.gen_bool(0.2) { 0 } else { range.gen_range(1..5000000) };
            let bond = match range.gen_range(0..3) {
                0 => 0,
                1 => initial_bond,
                2 => range.gen_range(1..5000000),
                _ => panic!("Invalid number generated"),
            };

            match range.gen_range(0..4) {
                0 => {
                    previous_roster.push(RosterEntry { pid: person.id, aid: aid_num });
                    current_database.push(person.clone());
                    last_query.push(RawInmate {
                        first_name: String::from("Testy"),
                        middle_name: i.to_string(),
                        last_name: String::from("McTesterface"),
                        age: Some(age as u8),
                        sex: Some(false),
                        height: Some(height),
                        weight: Some(weight),
                        bond: Some(bond),
                        birth_year: None,
                        arrest_date: Some(book_date),
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

                    arrests.push(Arrest {
                        pid: person.id,
                        id: aid_num,
                        booked: arrest_date,
                        agency_id: String::from("aa"),
                        bond: initial_bond,
                        initial_bond,
                        release_date: None,
                        holding_facility_id: String::new(),
                        notes: String::new(),
                        versioning: String::from("TESTING")
                    });

                    aid_num = aid_num + 1;
                },
                1 => {
                    previous_roster.push(RosterEntry { pid: person.id, aid: aid_num });

                    arrests.push(Arrest {
                        pid: person.id,
                        id: aid_num,
                        booked: arrest_date,
                        agency_id: String::from("aa"),
                        bond: initial_bond,
                        initial_bond,
                        release_date: None,
                        holding_facility_id: String::new(),
                        notes: String::new(),
                        versioning: String::from("TESTING")
                    });

                    current_database.push(person);
                    aid_num = aid_num + 1;
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

            println!("Done");
        }

        (people, previous_roster, current_database, last_query, arrests)
    }
}
