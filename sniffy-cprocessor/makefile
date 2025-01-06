module_compile_flags := 
CXX_STD := c++23
C_STD := c23
dir := $(shell pwd)
out := $(dir)/out
src := $(dir)/src
lib := $(dir)/lib
inc := $(dir)/include
iflags := -I$(inc) -Wall -D_POSIX_C_SOURCE=200809L -fdiagnostics-show-category=name

all: src

src: $(inc)/global.h $(inc)/rapidjson sniffy.out

clean:
	rm -rf sniffy.out $(out) $(lib) $(inc) $(lib)

sniffy.out: $(out)/main.cpp.o $(out)/listener.c.o $(out)/socket-handler.cpp.o $(out)/processor.cpp.o $(out)/arrest.cpp.o $(out)/person.cpp.o $(out)/sex.c.o $(out)/race.c.o $(out)/stringification.cpp.o $(out)/logging.c.o $(out)/charge.cpp.o $(out)/database.c.o
	clang++ $^ -o $@ $(iflags) -lcrypto -lmysqlclient

$(out):
	mkdir -p $@

$(lib):
	mkdir -p $@

$(inc):
	mkdir -p $@

$(inc)/rapidjson: $(inc) $(lib)/rapidjson
	cp -r $(lib)/rapidjson/include/rapidjson $@

$(inc)/global.h: ../global.h.in $(inc)
	cp $< $@ # This should be a command to generate that file eventually

$(lib)/rapidjson: $(lib)
	git -C $(lib) clone https://github.com/Tencent/rapidjson

$(out)/%.c.o: $(src)/%.c $(out)
	clang -c -o $@ $< $(module_compile_flags) -std=$(C_STD) $(iflags)

$(out)/%.cpp.o: $(src)/%.cpp $(out)
	clang++ -c -o $@ $< $(module_compile_flags) -std=$(CXX_STD) $(iflags)
