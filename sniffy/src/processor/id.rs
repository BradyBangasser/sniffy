pub mod id {
    // I might rewrite these functions in fortran for fun
    use crate::types::types::RawInmate;
    use std::format;
    use sha2::{Digest, Sha256};
    use chrono::Datelike;
    use regex::Regex;
    use std::ops::Deref;
    
    pub fn compute_id(inmate: &RawInmate) -> Option<[u8; 32]> {
        if inmate.birth_year.is_none() || inmate.sex.is_none() {
            return None;
        }

        _compute_id(inmate.last_name.to_lowercase(), inmate.first_name.to_lowercase(), inmate.sex.unwrap(), inmate.middle_name.to_lowercase(), inmate.birth_year.unwrap().year())
    }

    pub fn _compute_id(last: String, first: String, sex: bool, middle: String, birth_year: i32) -> Option<[u8; 32]> {
        let mut s256 = Sha256::new();
        let id_str = format!("{}:{}:{}:{}:{}", last.to_lowercase(), first.to_lowercase(), sex, middle.to_lowercase(), birth_year);
        s256.update(id_str);

        return Some(s256.finalize().into());
    }

    pub fn compute_agency_id(agency: &str) -> String {
        let re_special_chars = Regex::new("[^ 0-9a-zA-Z]").expect("Invalid Regex");
        let a = re_special_chars.replace_all(agency, "");
        let re_spaces = Regex::new("[^w]").expect("Invalid Regex");
        return re_spaces.replace_all(a.deref(), "_").to_string();
    }

    pub fn compute_facility_id(facility: &str) -> String {
        let re_special_chars = Regex::new("[^ 0-9a-zA-Z]").expect("Invalid Regex");
        let a = re_special_chars.replace_all(facility, "");
        let re_spaces = Regex::new("[^w]").expect("Invalid Regex");
        return re_spaces.replace_all(a.deref(), "_").to_string();
    }
}
