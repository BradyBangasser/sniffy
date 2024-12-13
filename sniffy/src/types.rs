pub mod types {
    use crate::preprocessor::parsers::parsers;
    use chrono::{DateTime,Utc};

    type Bond = u64;
    type Age = u8;
    type Height = u8;
    type Weight = u16;
    type Sex = bool;

    #[derive(Debug)]
    pub struct RawInmate {
        pub first_name: String,
        pub middle_name: String,
        pub last_name: String,
        pub age: Option<Age>,
        pub sex: Option<Sex>,
        pub height: Option<Height>,
        pub weight: Option<Weight>,
        pub bond: Option<Bond>,
        pub birth_year: Option<chrono::DateTime<chrono::Utc>>,
        pub arrest_date: Option<chrono::DateTime<chrono::Utc>>,
        pub arresting_agency: Option<String>,
        pub booking_agency: Option<String>,
        pub race: Option<String>,
        pub arrest_notes: String,
        pub holding_facility: Option<String>,
        pub release_date: Option<chrono::DateTime<chrono::Utc>>,
        pub court_date: Option<chrono::DateTime<chrono::Utc>>,
        pub image_id: Option<String>,
        pub home_address: Option<String>,
        pub case_id: Option<String>,
        pub charges: Vec<RawCharge>,
        pub notes: String
    }

    impl RawInmate {
        pub fn new() -> Self {
            return Self {
                first_name: String::new(),
                last_name: String::new(),
                middle_name: String::new(),
                age: None,
                sex: None,
                height: None,
                weight: None,
                bond: None,
                birth_year: None,
                arrest_date: None,
                arresting_agency: None,
                booking_agency: None,
                race: None,
                arrest_notes: String::new(),
                holding_facility: None,
                release_date: None,
                court_date: None,
                image_id: None,
                home_address: None,
                case_id: None,
                charges: Vec::new(),
                notes: String::new()
            }
        }
    }

    #[derive(Debug)]
    pub struct RawCharge {
        pub bond: Option<u64>,
        pub bond_status: Option<String>,
        pub datetime: Option<chrono::DateTime<chrono::Utc>>,
        pub statute_id: Option<String>,
        pub statute: Option<String>,
        pub statute_description: Option<String>,
        pub docket_number: Option<String>,
        pub timestamp: Option<DateTime<Utc>>,
        pub notes: String
    }

    impl RawCharge {
        pub fn new() -> Self {
            Self {
                bond: None,
                bond_status: None,
                datetime: None,
                statute_id: None,
                statute: None,
                statute_description: None,
                docket_number: None,
                timestamp: None,
                notes: String::new()
            }
        }

        pub fn parse_serde_charges(v: &mut Vec<Self>, value: &serde_json::Value) {
            if !value.is_array() {
                return;
            }

            let charges = value.as_array();

            for charge in charges.unwrap() {
                let parsed_charge = Self::parse_serde(charge);

                if parsed_charge.is_some() {
                    v.push(parsed_charge.unwrap());
                }
            }
        }

        pub fn parse_serde(value: &serde_json::Value) -> Option<Self> {
            if !value.is_object() {
                return None;
            }

            let obj = value.as_object().unwrap();
            let mut charge = Self::new();

            for key in obj.keys() {
                match key.as_str() {
                    "bond"|"bondamount" => charge.bond = obj[key].as_u64(),
                    "bondstatus" => charge.bond_status = Some(obj[key].to_string()),
                    "date" => charge.datetime = parsers::parse_serde_date(&obj[key]),
                    "name" => charge.statute_id = parsers::parse_serde_not_empty_string(&obj[key]),
                    "description" => charge.statute = parsers::parse_serde_not_empty_string(&obj[key]),
                    "docketnumber" => charge.docket_number = parsers::parse_serde_not_empty_string(&obj[key]),
                    _ => {
                        let v = parsers::parse_serde_not_empty_as_string(&obj[key]);

                        if v.is_some() {
                            charge.notes.push_str(key);
                            charge.notes.push_str(": ");
                            charge.notes.push_str(&v.unwrap());
                            charge.notes.push('\n');
                        }
                    }
                };
            }

            return Some(charge);
        }
    }

    #[derive(Debug)]
    pub struct Person {
        pub id: [u8; 32],
        pub first_name: String,
        pub middle_name: String,
        pub last_name: String,
        pub sex: bool,
        pub height: u8,
        pub weight: u16,
        pub birth_year: chrono::DateTime<chrono::Utc>,
        pub race: String,
        pub added: DateTime<Utc>,
        pub updated: DateTime<Utc>,
        pub notes: String,
        pub phone_number: Option<u32>,
        pub home_address: Option<String>,
        pub versioning: String,
    }

    #[derive(Debug)]
    pub struct Arrest {
        pub pid: [u8; 32],
        pub id: u64,
        pub booked: DateTime<Utc>,
        pub agency_id: String,
        pub bond: Bond,
        pub initial_bond: Bond,
        pub release_date: Option<DateTime<Utc>>,
        pub holding_facility_id: String,
        pub notes: String,
        pub versioning: String,
    }

    #[derive(Debug)]
    pub struct Charge {
        pub id: u64,
        pub aid: u64,
        pub sid: String,
        pub bond: Bond,
        pub initial_bond: Bond,
        pub bond_status: String,
        pub timestamp: DateTime<Utc>,
        pub docket_number: Option<String>,
        pub court_date: Option<DateTime<Utc>>,
        pub notes: String,
        pub versioning: String,
    }

    #[derive(Debug)]
    pub struct Statute {
        pub id: String,
        pub human_id: String,
        pub description: String,
        pub notes: String,
        pub versioning: String,
    }

    #[derive(Debug)]
    pub struct Mugshot {
        pub id: u64, // Same as Arrest ID
        pub pid: [u8; 32],
        pub data: Option<Vec<u8>>,
        pub url: Option<String>,
        pub format: u8,
        pub notes: String,
        pub versioning: String,
    }

    #[derive(Debug)]
    pub struct Agency {
        pub id: String,
        pub notes: String,
        pub versioning: String,
        pub title: String,
    }

    #[derive(Debug)]
    pub struct RosterEntry {
        pub pid: [u8; 32],
        pub aid: u64
    }
}
