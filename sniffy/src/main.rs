use std::os::unix::net::UnixStream;
use std::io::prelude::*;
pub mod types;
pub mod preprocessor;
fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut stream = UnixStream::connect("/tmp/sniffy.socket")?;
    Ok(())
}
