module_compile_flags := 
CXX_STD := c++23
C_STD := c23
out := out
src := src
lib := lib
inc := include
dir := $(shell pwd)

all: src

src: sniffy.out

clean:
	rm -rf sniffy.out $(out) $(lib)

sniffy.out: $(out)/main.cpp.o $(out)/listener.c.o
	clang++ $^ -o $@

$(out):
	mkdir -p $@

$(lib):
	mkdir -p $@

$(inc):
	mkdir -p $@

$(inc)/rapidjson: $(inc) $(lib)/rapidjson
	cp -r $(lib)/rapidjson/include $@

$(lib)/rapidjson: $(lib)
	git -C $(lib) clone https://github.com/Tencent/rapidjson

$(out)/%.c.o: $(src)/%.c $(out)
	clang -c -o $@ $< $(module_compile_flags) -std=$(C_STD)

$(out)/%.cpp.o: $(src)/%.cpp $(out)
	clang++ -c -o $@ $< $(module_compile_flags) -std=$(CXX_STD)
