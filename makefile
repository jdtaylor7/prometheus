# Detect Linux.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	CFLAGS += -D LINUX
	LINKOPTS += -L$(glfw)/lib/linux
	INCLUDES += -I/usr/include/libusb-1.0
endif
# ifeq ($(filter %CYGWIN, $(UNAME_S)),)
# 	CFLAGS += -D CYGWIN
# 	LINKOPTS += -L$(glfw)/lib/cygwin
# endif

# Detect architecture for Linux.
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M), x86_64)
	CFLAGS += -D AMD64
endif
ifneq ($(filter %86, $(UNAME_M)),)
	CFLAGS += -D IA32
endif

gl = third_party/GL
glad = third_party/glad
glfw = third_party/glfw-3.3.2
stb = third_party/stb_image
glm = third_party/glm-0.9.9.8

imgui = third_party/imgui
implot = third_party/implot

CC = clang
CXX = clang++
CFLAGS += -O2
CXXFLAGS = $(CFLAGS) -std=c++17
LINKOPTS += -L$(gl)/lib -lGL -lglfw3 -Wl,-Bstatic -lrt \
-Wl,-Bdynamic -lm -ldl -lX11 -lpthread
INCLUDES += -I$(gl) -I$(glad)/include -I$(glfw)/include -I$(stb) -I$(glm) \
-I$(implot) -Iinclude -I$(imgui) -I$(imgui)/examples -Iinclude/data \
-Iinclude/managers -Iinclude/misc
CXXFLAGS += -DIMGUI_IMPL_OPENGL_LOADER_GLAD
IMGUI_OBJS = imgui_impl_glfw.o imgui_impl_opengl3.o imgui_demo.o \
imgui_widgets.o imgui_draw.o imgui.o implot.o implot_demo.o com_port.o

all: main

glad.o: $(glad)/src/glad.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $^

stb_image.o: $(stb)/stb_image.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $^

imgui_impl_glfw.o: $(imgui)/examples/imgui_impl_glfw.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $^ $(LINKOPTS)

imgui_impl_opengl3.o: $(imgui)/examples/imgui_impl_opengl3.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $^ $(LINKOPTS)

imgui_demo.o: $(imgui)/imgui_demo.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $^

imgui_widgets.o: $(imgui)/imgui_widgets.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $^

imgui_draw.o: $(imgui)/imgui_draw.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $^

imgui.o: glad.o $(imgui)/imgui.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $^

implot.o: imgui.o $(implot)/implot.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $^

implot_demo.o: $(implot)/implot_demo.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $^

com_port.o: src/com_port.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $^

main: glad.o stb_image.o $(IMGUI_OBJS) src/main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o main $(LINKOPTS)

clean:
	rm -vf *.o *.exe *.gch *.exe.stackdump

clean_main:
	rm -vf *.exe *.gch *.exe.stackdump imgui.imi
