gl = third_party/GL
glad = third_party/glad
glfw = third_party/glfw-3.3.2
stb = third_party/stb_image
glm = third_party/glm-0.9.9.8

imgui = third_party/imgui

CFLAGS = -O2
CXXFLAGS = -std=c++17 -O2
LINKOPTS = -L$(gl)/lib -L$(glfw)/build/lib -lGL -lglfw3 -Wl,-Bstatic -lm -lrt -Wl,-Bdynamic -ldl -lX11
includes = -I$(gl) -I$(glad)/include -I$(glfw)/include -I$(stb) -I$(glm) -Iinclude
includes += -I$(imgui) -I$(imgui)/examples
CXXFLAGS += -DIMGUI_IMPL_OPENGL_LOADER_GLAD
IMGUI_OBJS = imgui_impl_glfw.o imgui_impl_opengl3.o imgui_demo.o imgui_widgets.o imgui_draw.o imgui.o

all: main

glad.o: $(glad)/src/glad.c
	clang -c $(CFLAGS) $(includes) $^

stb_image.o: $(stb)/stb_image.c
	clang -c $(CFLAGS) $(includes) $^

imgui_impl_glfw.o: $(imgui)/examples/imgui_impl_glfw.cpp
	clang++ -c $(CXXFLAGS) $(includes) $^ $(LINKOPTS)

imgui_impl_opengl3.o: $(imgui)/examples/imgui_impl_opengl3.cpp
	clang++ -c $(CXXFLAGS) $(includes) $^ $(LINKOPTS)

imgui_demo.o: $(imgui)/imgui_demo.cpp
	clang++ -c $(CXXFLAGS) $(includes) $^

imgui_widgets.o: $(imgui)/imgui_widgets.cpp
	clang++ -c $(CXXFLAGS) $(includes) $^

imgui_draw.o: $(imgui)/imgui_draw.cpp
	clang++ -c $(CXXFLAGS) $(includes) $^

imgui.o: glad.o $(imgui)/imgui.cpp
	clang++ -c $(CXXFLAGS) $(includes) $^

main: glad.o stb_image.o $(IMGUI_OBJS) src/main.cpp
	clang++ $(CXXFLAGS) $(includes)	$^ -o main.exe $(LINKOPTS)

triangle: glad.o $(IMGUI_OBJS) src/triangle.cpp
	clang++ $(CXXFLAGS) $(includes) $^ -o triangle.exe $(LINKOPTS)

clean:
	rm -vf *.o *.exe *.gch *.exe.stackdump

clean_main:
	rm -vf *.exe *.gch *.exe.stackdump
