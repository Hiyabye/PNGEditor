# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(PNGEditor)

# Build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug/Release)" FORCE)
endif()

# Compiler flags
add_executable(PhotoEditor src/main.cpp src/image.cpp src/render.cpp)
target_compile_features(PhotoEditor PRIVATE cxx_std_17)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
add_subdirectory(lib/glfw EXCLUDE_FROM_ALL)
target_link_libraries(PhotoEditor glfw)

# GLAD
add_library(glad lib/glad/src/glad.c)
target_include_directories(glad PUBLIC lib/glad/include)
target_link_libraries(PhotoEditor glad ${CMAKE_DL_LIBS})

# Dear ImGui
target_compile_definitions(PhotoEditor PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD2)
include_directories(lib/imgui)
include_directories(lib/imgui/backends)
include_directories(lib/imgui/misc/cpp) # For std::string support
add_library(imgui
  lib/imgui/imgui.cpp
  lib/imgui/imgui_demo.cpp
  lib/imgui/imgui_draw.cpp
  lib/imgui/imgui_tables.cpp
  lib/imgui/imgui_widgets.cpp
  lib/imgui/backends/imgui_impl_glfw.cpp
  lib/imgui/backends/imgui_impl_opengl3.cpp
)
target_compile_features(imgui PRIVATE cxx_std_17)
target_include_directories(imgui PUBLIC lib/glfw/include)
target_link_libraries(PhotoEditor imgui)

# imgui-filebrowser
include_directories(lib/imgui-filebrowser)

# libpng
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/lib/libpng" CACHE PATH "Install path" FORCE)
set(PNG_TESTS OFF CACHE BOOL "Build libpng tests" FORCE)
set(PNG_SHARED OFF CACHE BOOL "Build libpng shared library" FORCE)
set(PNG_STATIC ON CACHE BOOL "Build libpng static library" FORCE)
set(PNG_DEBUG OFF CACHE BOOL "Build libpng with debug info" FORCE)
set(PNGARG OFF CACHE BOOL "Enable libpng arguments" FORCE)
add_subdirectory(lib/libpng EXCLUDE_FROM_ALL)
target_include_directories(PhotoEditor PUBLIC
  lib/libpng
  lib/libpng/contrib/libtests
  lib/libpng/contrib/tools
  "${CMAKE_BINARY_DIR}/lib/libpng"
)
target_link_libraries(PhotoEditor png_static)

# Assets
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})