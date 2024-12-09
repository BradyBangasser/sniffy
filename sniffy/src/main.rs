mod preprocessor;
mod processor;
mod database;
mod module_loader;
mod stream;
pub mod types;

use std::ptr::null;

fn main() {
    let mut pp = crate::preprocessor::preprocessor::Preprocessor::new();    
    let mut nmods: u16 = 0;
    let mut omod = module_loader::ModuleOut {
        enc: 0,
        out: null()
    };

    let m = unsafe { std::slice::from_raw_parts(module_loader::load_modules(&mut nmods), nmods as usize) };

    unsafe {
        for i in m {
            println!("{:#?}", std::ffi::CStr::from_bytes_until_nul(&i.module_path).unwrap());
            module_loader::exec_module(0, &mut omod);

            pp.push(std::ffi::CStr::from_ptr(omod.out).to_str().unwrap().to_string());
        }
    }
  pp.despool();
}
