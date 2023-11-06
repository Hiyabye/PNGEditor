#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "png.h"

bool load_image(const std::string& filename, ImVec2& image_size, ImTextureID& image_texture) {
  // Open the PNG file
  FILE* file = nullptr;
  try {
    file = fopen(filename.c_str(), "rb");
  } catch (const std::exception& e) {
    std::cerr << "Failed to open PNG file: " << filename << std::endl;
    return false;
  }

  png_structp png = nullptr;
  try {
    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  } catch (const std::exception& e) {
    std::cerr << "Failed to create PNG read struct" << std::endl;
    fclose(file);
    return false;
  }

  png_infop info = nullptr;
  try {
    info = png_create_info_struct(png);
  } catch (const std::exception& e) {
    std::cerr << "Failed to create PNG info struct" << std::endl;
    png_destroy_read_struct(&png, nullptr, nullptr);
    fclose(file);
    return false;
  }

  if (setjmp(png_jmpbuf(png))) {
    std::cerr << "Failed to set PNG jump buffer" << std::endl;
    png_destroy_read_struct(&png, &info, nullptr);
    fclose(file);
    return false;
  }

  png_init_io(png, file);
  png_read_info(png, info);

  int width = png_get_image_width(png, info);
  int height = png_get_image_height(png, info);
  int bit_depth = png_get_bit_depth(png, info);
  int color_type = png_get_color_type(png, info);

  // Read the PNG image
  png_bytep* row_pointers = nullptr;
  png_byte* image_data = nullptr;

  // Handle different color types
  switch (color_type) {
    case PNG_COLOR_TYPE_RGBA:
      std::cout << "RGBA image" << std::endl;

      // Create a buffer to hold the image data
      row_pointers = new png_bytep[height];
      for (int i = 0; i < height; ++i) row_pointers[i] = new png_byte[png_get_rowbytes(png, info)];
      png_read_image(png, row_pointers);

      // Flatten the image data
      image_data = new png_byte[width * height * 4];
      for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
          image_data[(i * width + j) * 4 + 0] = row_pointers[i][j * 4 + 0];
          image_data[(i * width + j) * 4 + 1] = row_pointers[i][j * 4 + 1];
          image_data[(i * width + j) * 4 + 2] = row_pointers[i][j * 4 + 2];
          image_data[(i * width + j) * 4 + 3] = row_pointers[i][j * 4 + 3];
        }
      }

      // Create an OpenGL texture from the image data
      GLuint textureID;
      glGenTextures(1, &textureID);
      glBindTexture(GL_TEXTURE_2D, textureID);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      image_texture = reinterpret_cast<ImTextureID>(static_cast<intptr_t>(textureID));

      // Set the image size
      image_size = ImVec2(static_cast<float>(width), static_cast<float>(height));

      // Clean up
      for (int i = 0; i < height; ++i) delete[] row_pointers[i];
      delete[] row_pointers;
      delete[] image_data;
      break;

    default:
      // Unsupported color type
      std::cerr << "Unsupported PNG color type: " << color_type << std::endl;
      break;
  }

  // Clean up
  png_destroy_read_struct(&png, &info, nullptr);
  fclose(file);

  return true;
}

int main(int argc, char *argv[]) {
  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Error: Failed to initialize GLFW" << std::endl;
    return 1;
  }

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char* glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

  // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(1280, 720, "PNGEditor", nullptr, nullptr);
  if (window == nullptr) {
    std::cout << "Error: Failed to create GLFW window" << std::endl;
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Initialize OpenGL loader
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Error: Failed to initialize GLAD" << std::endl;
    return 1;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;  // Enable MousePos requests
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Poll and handle events (inputs, window resize, etc.)
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Variables
    static bool png_loaded = false;
    static ImTextureID png_texture;
    static ImVec2 image_size;

    // Main Menu
    {
      // Flags
      static ImGuiWindowFlags window_flags = 0;
      window_flags |= ImGuiWindowFlags_MenuBar;
      window_flags |= ImGuiWindowFlags_NoCollapse;

      // Start of window
      ImGui::Begin("Main Menu", nullptr, window_flags);

      // Menu bar
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          // TODO: Implement file menu
          if (ImGui::MenuItem("Open", "Ctrl+O")) {
            // TODO: Implement open file dialog

            // Load the PNG image
            png_loaded = load_image("../sphere.png", image_size, png_texture);
          }

          if (ImGui::MenuItem("Save", "Ctrl+S")) {}
          if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {}
          
          ImGui::Separator();
          if (ImGui::MenuItem("Quit", "Ctrl+Q")) {
            glfwSetWindowShouldClose(window, true);
          }

          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }

      if (!png_loaded) {
        ImGui::Text("No PNG file loaded");
      }

      // End of window
      ImGui::End();
    }

    // PNG Editor
    if (png_loaded) {
      // Flags
      static ImGuiWindowFlags window_flags = 0;

      // Set the window size to the image size
      ImGui::SetNextWindowSize(image_size);

      // Start of window
      ImGui::Begin("PNG Editor", nullptr, window_flags);

      // Display the loaded PNG image
      ImGui::Image(png_texture, image_size);

      // End of window
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
