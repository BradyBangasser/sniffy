extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    // /usr/lib/x86_64-linux-gnu
    println!("cargo:rustc-link-search=native=target/lib");
    println!("cargo:rustc-link-search=native=/usr/lib/x86_64-linux-gnu");
    println!("cargo:rustc-link-lib=module_loader");
    println!("cargo:rustc-link-lib=stdc++");
    println!("cargo:rustc-link-lib=lua5.4");
    let bindings = bindgen::Builder::default()
        .header("modules/module_loader.h")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .generate()
        .expect("I can't generate bindings :(");

    let out_binding = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_binding.join("bindings.rs"))
        .expect("Failed to write bindings");
}
