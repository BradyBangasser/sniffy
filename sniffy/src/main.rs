use std::os::unix::net::UnixStream;
use std::io::prelude::*;
pub mod types;
pub mod preprocessor;
pub mod module_loader;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut stream = UnixStream::connect("/tmp/sniffy.socket")?;
    let mut nmods = 0;
    let m = unsafe { std::slice::from_raw_parts(module_loader::load_modules(&mut nmods), nmods as usize) };
    let mut out = module_loader::ModuleOut {
        enc: 0,
        out: std::ptr::null(),
    };

    unsafe {
        for module in m {
            println!("Executing module {:?}", std::ffi::CStr::from_bytes_until_nul(&module.module_path)?);
            module_loader::exec_module(module.id, &mut out);
            let mut str = String::from(std::ffi::CStr::from_ptr(out.out).to_str().unwrap());
            str = str.to_lowercase();
            let json = str.into_bytes();

            let _ = stream.write_all(&json.len().to_ne_bytes());
            let _ = stream.write_all(&json);
        }
    }
    Ok(())
}
