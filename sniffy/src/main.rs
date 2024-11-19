pub mod preprocesser;
mod types;

use preprocessor::preprocessor;

fn main() {
    preprocessor::Preprocessor::new();    
    println!("Hello, world!");
}
