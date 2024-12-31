module_compile_flags := 
CXX_STD := c++23
C_STD := c23
dir := $(shell pwd)
out := $(dir)/out
src := $(dir)/src
lib := $(dir)/lib
inc := $(dir)/include
iflags := -I$(inc)

all: src

src: $(inc)/rapidjson sniffy.out

clean:
	rm -rf sniffy.out $(out) $(lib) $(inc) $(lib)

sniffy.out: $(out)/main.cpp.o $(out)/listener.c.o $(out)/socket-handler.cpp.o $(out)/processor.cpp.o $(out)/arrest.cpp.o
	clang++ $^ -o $@ $(iflags)

$(out):
	mkdir -p $@

$(lib):
	mkdir -p $@

$(inc):
	mkdir -p $@

$(inc)/rapidjson: $(inc) $(lib)/rapidjson
	cp -r $(lib)/rapidjson/include/rapidjson $@

$(lib)/rapidjson: $(lib)
	git -C $(lib) clone https://github.com/Tencent/rapidjson

$(out)/%.c.o: $(src)/%.c $(out)
	clang -c -o $@ $< $(module_compile_flags) -std=$(C_STD) $(iflags)

$(out)/%.cpp.o: $(src)/%.cpp $(out)
	clang++ -c -o $@ $< $(module_compile_flags) -std=$(CXX_STD) $(iflags)
