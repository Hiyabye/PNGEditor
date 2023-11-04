# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(PNGEditor)

# Build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# Compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Main executable
add_executable(PhotoEditor src/main.cpp)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)
target_link_libraries(PhotoEditor glfw)

# GLAD
set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
add_library(glad lib/glad/src/glad.c)
target_include_directories(glad PUBLIC lib/glad/include)
target_link_libraries(PhotoEditor glad ${CMAKE_DL_LIBS})

# Dear ImGui
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIMGUI_IMPL_OPENGL_LOADER_GLAD2")
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
target_include_directories(imgui PUBLIC lib/glfw/include)
target_link_libraries(PhotoEditor imgui ${CMAKE_DL_LIBS})

# libpng
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/lib/libpng")
set(PNG_TESTS OFF CACHE BOOL "" FORCE)
set(PNG_SHARED OFF CACHE BOOL "" FORCE)
set(PNG_STATIC ON CACHE BOOL "" FORCE)
set(PNG_DEBUG OFF CACHE BOOL "" FORCE)
set(PNGARG OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/libpng)
target_include_directories(PhotoEditor PUBLIC lib/libpng)
target_include_directories(PhotoEditor PUBLIC lib/libpng/contrib/libtests)
target_include_directories(PhotoEditor PUBLIC lib/libpng/contrib/tools)
target_include_directories(PhotoEditor PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/lib/libpng")
target_link_libraries(PhotoEditor png_static)