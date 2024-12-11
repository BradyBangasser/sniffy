pub mod id {
    // I might rewrite these functions in fortran for fun
    use crate::types::types::RawInmate;
    use std::format;
    use sha2::{Digest, Sha256};
    use chrono::Datelike;
    
    pub fn compute_id(inmate: &RawInmate) -> Option<[u8; 32]> {
        if inmate.birth_year.is_none() || inmate.sex.is_none() {
            return None;
        }

        let id_str = format!("{}:{}:{}:{}:{}", inmate.last_name.to_lowercase(), inmate.first_name.to_lowercase(), inmate.sex.unwrap(), inmate.middle_name.to_lowercase(), inmate.birth_year.unwrap().year());

        let mut s256 = Sha256::new();
        s256.update(id_str);

        return Some(s256.finalize().into());
    }
}
