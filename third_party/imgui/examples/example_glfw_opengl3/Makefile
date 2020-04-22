#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need GLFW (http://www.glfw.org):
# Linux:
#   apt-get install libglfw-dev
# Mac OS X:
#   brew install glfw
# MSYS2:
#   pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw
#

#CXX = g++
CXX = clang++

EXE = example_glfw_opengl3
SOURCES = main.cpp
SOURCES += ../imgui_impl_glfw.cpp ../imgui_impl_opengl3.cpp
SOURCES += ../../imgui.cpp ../../imgui_demo.cpp ../../imgui_draw.cpp ../../imgui_widgets.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)

CXXFLAGS = -I../ -I../../
CXXFLAGS += -g -Wall -Wformat
# LIBS =

##---------------------------------------------------------------------
## OPENGL LOADER
##---------------------------------------------------------------------

## Using OpenGL loader: glad
SOURCES += ../libs/glad/src/glad.c
CXXFLAGS += -I../../../glad/include -DIMGUI_IMPL_OPENGL_LOADER_GLAD
CFLAGS += -I../../../glad/include
CXXFLAGS += -I../../../glfw-3.3.2/include
CXXFLAGS += -L../../../GL/lib -L../../../glfw-3.3.2/build/lib
CXXFLAGS += -lGL -lglfw3 -Wl,-Bstatic -lm -lrt -Wl,-Bdynamic -ldl -lX11
CXXFLAGS += -v

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:../%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:../../%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:../libs/gl3w/GL/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o:../../../glad/src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -f $(EXE) $(OBJS)
