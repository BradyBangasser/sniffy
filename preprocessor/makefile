module_compile_flags := -I/usr/include/lua5.4.7
CXX_STD := c++23
C_STD := c23
out := target/lib

all: module

module: $(out)/libmodule_loader.a

clean:
	rm -rf target

$(out)/libmodule_loader.a: $(out)/modules/lua_loader.c.o $(out)/modules/module_loader.cpp.o 
	ar rcs $@ $^
	touch $(out)/global.h

$(out):
	mkdir -p $@

$(out)/modules: 
	mkdir -p $@

$(out)/global.h: ../global.h.in $(out)
	cp $< $@ # This should be a command to generate that file eventually

test:
	make all
	clang++ test.cpp -I/usr/include/lua5.3 -L/usr/lib/x86_64-linux-gnu -Ltarget/lib -lmodule_loader -std=$(CXX_STD) -llua5.3

$(out)/modules/%.c.o: modules/%.c $(out)/modules $(out)/global.h
	clang -c -o $@ $< $(module_compile_flags) -std=$(C_STD) -I$(out)

$(out)/modules/%.cpp.o: modules/%.cpp $(out)/modules $(out)/global.h
	clang++ -c -o $@ $< $(module_compile_flags) -std=$(CXX_STD) -I$(out)
