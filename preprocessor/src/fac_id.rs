pub fn generate_facility_id(address: &[u8], name: &[u8], _cap: u16) -> u32 {
    let mut id: u32 = 5381;


    for c in address {
        // I hate rust
        id = id.wrapping_mul(33);
        id = id.wrapping_add(*c as u32);
    }
    
    for c in name {
        id = id.wrapping_mul(33);
        id = id.wrapping_add(*c as u32);
    }

    return id;
}
