pub mod parsers { 
    use serde_json::Value;
    use chrono::{DateTime, Utc};

    pub fn parse_serde_not_empty_as_string(value: &Value) -> Option<String> {
        match value {
            Value::Null => None,
            Value::Bool(b) => Some(b.to_string()),
            Value::Number(_) => Some(value.to_string()),
            Value::Array(a) => {
                if a.len() > 0 {
                    return Some(value.to_string());
                } else {
                    None
                }
            }
            Value::Object(o) => {
                if o.is_empty() {
                    return None;
                } else {
                    return Some(value.to_string());
                }
            }
            Value::String(s) => {
                if s.len() > 0 {
                    return Some(s.to_string());
                } else {
                    return None;
                }
            }
        }
    }

    pub fn parse_serde_date(value: &Value) -> Option<DateTime<Utc>> {
        match value {
            Value::Number(n) => {
                DateTime::from_timestamp(n.as_i64().expect("Failed to parse epoch from json"), 0)
            },
            Value::String(s) => {
                let mut time = DateTime::parse_from_rfc2822(s);

                if time.is_err() {
                    time = DateTime::parse_from_rfc3339(s);
                }

                if time.is_ok() {
                    return Some(time.unwrap().into());
                }

                return None;
            },
            _ => None,
        }
    }

    pub fn parse_serde_sex(value: &Value) -> Option<bool> {
        match value {
            Value::Bool(v) => Some(*v),
            Value::String(v) => Some(v.eq("male")), // This needs to be better
            _ => None
        }
    }

    pub fn parse_serde_height(value: &Value) -> Option<u8> {
        match value {
            Value::Number(v) => {
                match v.as_u64() {
                    Some(n) => {
                        let h = n.try_into();

                        if h.is_ok() {
                            return Some(h.unwrap());
                        }

                        None
                    },

                    None => None
                }
            },
            Value::String(s) => {
                let bytes = s.as_bytes();
                if bytes[1] == '\'' as u8 {
                    let ft = s[0..1].parse::<u8>();
                    let inches = s[3..5].parse::<u8>();

                    if ft.is_err() || inches.is_err() {
                        return None;
                    }

                    Some(ft.unwrap() * 12 + inches.unwrap())
                } else {
                    let parsed_height = s.parse::<u8>();

                    if parsed_height.is_ok() {
                        return Some(parsed_height.unwrap())
                    } 

                    None
                }
            }

            _ => None
        }
    }

    pub fn parse_serde_weight(value: &Value) -> Option<u8> {
        match value {
            Value::String(v) => {
                let p = v.parse::<u8>();
                if p.is_ok() {
                    return Some(p.unwrap());
                }

                None
            }
            Value::Number(n) => {
                let n64 = n.as_u64();
                if n64.is_some() {
                    let n8 = n64.unwrap().try_into();
                    if n8.is_ok() {
                        return Some(n8.unwrap());
                    }
                }

                None
            }
            _ => None
        }
    }

    pub fn parse_serde_not_empty_string(value: &Value) -> Option<String> {
        let s = value.to_string();

        if s.len() > 0 {
            return Some(s);
        }

        None
    }
}
