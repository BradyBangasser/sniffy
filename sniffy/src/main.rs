mod preprocessor;
mod processor;
mod database;
mod module_loader;
mod stream;
mod test;
pub mod types;

use std::{ptr::null, sync::Arc};
use database::database::Connection;
use processor::processor::Processor;

use crate::stream::stream::Stream;
use crate::types::types::RawInmate;

fn main() {
    // let s = Stream::<RawInmate>::new();
    // let mut pp = crate::preprocessor::preprocessor::Preprocessor::new(Arc::clone(&s));    
    // let mut nmods: u16 = 0;
    // let mut omod = module_loader::ModuleOut {
    //     enc: 0,
    //     out: null()
    // };

    // let m = unsafe { std::slice::from_raw_parts(module_loader::load_modules(&mut nmods), nmods as usize) };

    // unsafe {
    //     for i in m {
    //         println!("{:#?}", std::ffi::CStr::from_bytes_until_nul(&i.module_path).unwrap());
    //         module_loader::exec_module(0, &mut omod);

    //         pp.push(std::ffi::CStr::from_ptr(omod.out).to_str().unwrap().to_string());
    //     }
    // }
    let test_data = test::test::generate_test_data();
    println!("HERE");
    let conn = Connection::new_test(test_data.1, test_data.2).expect("Failed to connect to database");
    let p = Processor::new(Arc::clone(&test_data.3), Arc::new(conn));
}
