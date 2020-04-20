gl = third_party/GL
glad = third_party/glad
glfw = third_party/glfw-3.3.2
stb = third_party/stb_image
glm = third_party/glm-0.9.9.8

CFLAGS = -O2
CXXFLAGS = -std=c++17 -O2
LINKOPTS = -L$(gl)/lib -L$(glfw)/build/lib -lGL -lglfw3 -Wl,-Bstatic -lm -lrt -Wl,-Bdynamic -ldl -lX11
includes = -I$(gl) -I$(glad)/include -I$(glfw)/include -I$(stb) -I$(glm) -Iinclude

all: main

glad.o: $(glad)/src/glad.c
	clang -c $(CFLAGS) $(includes) $^

stb_image.o: $(stb)/stb_image.c
	clang -c $(CFLAGS) $(includes) $^

main: glad.o stb_image.o src/main.cpp
	clang++ $(CXXFLAGS) $(includes)	$^ -o main.exe $(LINKOPTS)

clean:
	rm -vf *.o *.exe *.gch *.exe.stackdump

clean_main:
	rm -vf *.exe *.gch *.exe.stackdump
