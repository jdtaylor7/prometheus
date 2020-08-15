cmake_minimum_required(VERSION 3.14)

# Set project name.
project(DroneViewer VERSION 0.1.0)

# Specify C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Build type.
set(CMAKE_BUILD_TYPE Release)

# Create compilation database for Sourcetrail.
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

# Add packages.
set(CMAKE_PREFIX_PATH /usr/lib/glfw)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 CONFIG REQUIRED)
find_package(ASSIMP CONFIG REQUIRED)

# Find libraries.
find_library(LIBSERIAL_LIB serial "${PROJECT_SOURCE_DIR}/third_party/libserial/lib")

# Detect OS.
string(COMPARE EQUAL "${CMAKE_SYSTEM_NAME}" "Linux" is_linux)
if(is_linux)
    set(CXXOPTS "${CXXOPTS} -DOS_LINUX")
	include_directories(/usr/include/libusb-1.0)
elseif(CYGWIN)
    set(CXXOPTS "${CXXOPTS} -DOS_CYGWIN")
endif()

# Add include directories
include_directories(
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/third_party/GL"
    "${PROJECT_SOURCE_DIR}/third_party/glad/include"
    "${PROJECT_SOURCE_DIR}/third_party/glfw-3.3.2/include"
    "${PROJECT_SOURCE_DIR}/third_party/glm-0.9.9.8"
    "${PROJECT_SOURCE_DIR}/third_party/stb_image"
    "${PROJECT_SOURCE_DIR}/third_party/imgui"
    "${PROJECT_SOURCE_DIR}/third_party/imgui/examples"
    "${PROJECT_SOURCE_DIR}/third_party/implot"
    "${PROJECT_SOURCE_DIR}/third_party/libserial/include"
    "${PROJECT_SOURCE_DIR}/include/data"
    "${PROJECT_SOURCE_DIR}/include/drivers"
    "${PROJECT_SOURCE_DIR}/include/managers"
    "${PROJECT_SOURCE_DIR}/include/models"
    "${PROJECT_SOURCE_DIR}/include/misc"
)

# Add library directories.
link_directories(
    "${ASSIMP_LIBRARIES}"
)

# Add compiler/linker options.
set(CXXOPTS "${CXXOPTS} -DIMGUI_IMPL_OPENGL_LOADER_GLAD")
set(LINKOPTS "${LINKOPTS} -lGL -lglfw -Wl,-Bstatic -lm -lrt -Wl,-Bdynamic -ldl -lX11 -lpthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXXOPTS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKOPTS}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

# Add libraries.
add_library(glad OBJECT third_party/glad/src/glad.c)
add_library(stb_image OBJECT third_party/stb_image/stb_image.c)

add_library(imgui_impl_glfw OBJECT third_party/imgui/examples/imgui_impl_glfw.cpp)
add_library(imgui_impl_opengl3 OBJECT third_party/imgui/examples/imgui_impl_opengl3.cpp)
add_library(imgui_demo OBJECT third_party/imgui/imgui_demo.cpp)
add_library(imgui_widgets OBJECT third_party/imgui/imgui_widgets.cpp)
add_library(imgui_draw OBJECT third_party/imgui/imgui_draw.cpp)
add_library(imgui OBJECT third_party/imgui/imgui.cpp)

add_library(implot OBJECT third_party/implot/implot.cpp)
add_library(implot_demo OBJECT third_party/implot/implot_demo.cpp)

add_library(linux_serial_port OBJECT src/drivers/linux_serial_port.cpp)
add_library(windows_serial_port OBJECT src/drivers/windows_serial_port.cpp)
add_library(serial_port OBJECT src/drivers/serial_port.cpp)

# Add executables.
add_executable(prometheus src/prometheus.cpp)

# Link libraries.
target_link_libraries(prometheus
    glad
    stb_image
    glfw
    assimp
    "${OPENGL_LIBRARIES}"
    imgui_impl_glfw
    imgui_impl_opengl3
    imgui_demo
    imgui_widgets
    imgui_draw
    imgui
    implot
    implot_demo
    "${LIBSERIAL_LIB}"
    linux_serial_port
    windows_serial_port
    serial_port
    dl
    pthread
)
