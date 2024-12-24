module_compile_flags := -I/usr/include/lua5.4.7
CXX_STD := c++23
C_STD := c23
out := out
src := src

all: src

src: sniffy.out

clean:
	rm -rf target

sniffy.out: $(out)/main.cpp.o 
	clang++ $^ -o $@

$(out):
	mkdir -p $@

$(out)/%.c.o: $(src)/%.c $(out)
	clang -c -o $@ $< $(module_compile_flags) -std=$(C_STD)

$(out)/%.cpp.o: $(src)/%.cpp $(out)
	clang++ -c -o $@ $< $(module_compile_flags) -std=$(CXX_STD)
