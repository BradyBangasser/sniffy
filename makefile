all: preprocessor processor

clean:
	cd preprocessor && make clean
	cd processor && make clean

preprocessor:
	cd preprocessor && make && cargo build
processor: 
	cd processor && make

.PHONY: all clean preprocessor processor
