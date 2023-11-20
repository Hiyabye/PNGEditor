# PNG Editor

This is a simple PNG editor created as part of a course project for CSE2035 at Sogang University.
It is developed in C++17 and uses the libpng library for reading and writing PNG files, as well as Dear ImGui for rendering.

## Prerequisites

 - zlib: libpng dependency
 - CMake: build system
 - Make: build system
 - C++17 compiler: build system

## Installation

Follow these steps to install the executable on your system:

1. Clone the repository, including its submodules, by running the following command in your terminal:
```bash
git clone https://github.com/Hiyabye/PNGEditor.git --recurse-submodules
```

2. Build the project with CMake:
```bash
cd PNGEditor
mkdir build
cd build
cmake ..
make
```

## Acknowledgements

 - [ChatGPT](https://chat.openai.com/): All documentation was written using ChatGPT.
 - [gitignore](https://www.toptal.com/developers/gitignore)
 - [LearnOpenGL](https://learnopengl.com/)
 - [glad](https://glad.dav1d.de/)
 - [glfw](https://www.glfw.org/)
 - [libpng](http://www.libpng.org/pub/png/libpng.html)
 - [Dear ImGui](https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/main.cpp)
 - [imgui-filebrowser](https://github.com/AirGuanZ/imgui-filebrowser)
 - [Icons8](https://icons8.com/)

## License

This project is licensed under the [MIT License](https://choosealicense.com/licenses/mit/).
For more details, please refer to the [LICENSE](LICENSE) file.
