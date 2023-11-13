#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imfilebrowser.h"
#include "png.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

std::vector<png_byte> imageData;
int imageWidth = 0, imageHeight = 0;
bool pngLoaded = false, openDialog = false;
ImTextureID imageTexture;

void createOpenGLTexture(void) {
  // Create an OpenGL texture from the image data
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
  imageTexture = reinterpret_cast<ImTextureID>(static_cast<intptr_t>(textureID));
}

void updateOpenGLTexture(void) {
  glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)imageTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
}

bool loadImage(const std::string& filename) {
  // Open the PNG file
  FILE* file = fopen(filename.c_str(), "rb");
  if (!file) {
    std::cerr << "Failed to open for reading: " << filename << std::endl;
    return false;
  }
  std::unique_ptr<FILE, decltype(&fclose)> fileGuard(file, fclose);

  // Create a PNG read struct
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png) {
    std::cerr << "Failed to create PNG read struct" << std::endl;
    return false;
  }

  // Create a PNG info struct
  png_infop info = png_create_info_struct(png);
  if (!info) {
    std::cerr << "Failed to create PNG info struct" << std::endl;
    png_destroy_read_struct(&png, nullptr, nullptr);
    return false;
  }

  // Set the PNG jump buffer
  if (setjmp(png_jmpbuf(png))) {
    std::cerr << "Failed to set PNG jump buffer" << std::endl;
    png_destroy_read_struct(&png, &info, nullptr);
    return false;
  }

  png_init_io(png, file);
  png_read_info(png, info);

  imageWidth = png_get_image_width(png, info);
  imageHeight = png_get_image_height(png, info);
  int bitDepth = png_get_bit_depth(png, info);
  int colorType = png_get_color_type(png, info);

  // Read the PNG image
  png_bytep* rowPointers = nullptr;
  imageData.resize(imageWidth * imageHeight * 4);

  // Handle different color types
  switch (colorType) {
    case PNG_COLOR_TYPE_RGBA: // RGBA
      // Create a buffer to hold the image data
      rowPointers = new png_bytep[imageHeight];
      for (int i = 0; i < imageHeight; ++i) rowPointers[i] = new png_byte[png_get_rowbytes(png, info)];
      png_read_image(png, rowPointers);

      // Flatten the image data
      for (int i = 0; i < imageHeight; ++i) {
        for (int j = 0; j < imageWidth; ++j) {
          imageData[(i * imageWidth + j) * 4 + 0] = rowPointers[i][j * 4 + 0];
          imageData[(i * imageWidth + j) * 4 + 1] = rowPointers[i][j * 4 + 1];
          imageData[(i * imageWidth + j) * 4 + 2] = rowPointers[i][j * 4 + 2];
          imageData[(i * imageWidth + j) * 4 + 3] = rowPointers[i][j * 4 + 3];
        }
      }
      createOpenGLTexture();

      // Clean up
      for (int i = 0; i < imageHeight; ++i) delete[] rowPointers[i];
      delete[] rowPointers;
      break;

    default:
      // Unsupported color type
      std::cerr << "Unsupported PNG color type: " << colorType << std::endl;
      break;
  }
  
  png_destroy_read_struct(&png, &info, nullptr);
  return true;
}

bool saveImage(const std::string& filename) {
  // Open the PNG file
  FILE* file = fopen(filename.c_str(), "wb");
  if (!file) {
    std::cerr << "Failed to open for writing: " << filename << std::endl;
    return false;
  }
  std::unique_ptr<FILE, decltype(&fclose)> fileGuard(file, fclose);

  // Create a PNG write struct
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png) {
    std::cerr << "Failed to create PNG write struct" << std::endl;
    return false;
  }

  // Create a PNG info struct
  png_infop info = png_create_info_struct(png);
  if (!info) {
    std::cerr << "Failed to create PNG info struct" << std::endl;
    png_destroy_write_struct(&png, (png_infopp)nullptr);
    return false;
  }

  // Set the error handler
  if (setjmp(png_jmpbuf(png))) {
    std::cerr << "Failed to set PNG jump buffer" << std::endl;
    png_destroy_write_struct(&png, &info);
    return false;
  }

  // Write the PNG image
  png_init_io(png, file);
  png_set_IHDR(png, info, imageWidth, imageHeight, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  std::vector<png_bytep> rowPointers(imageHeight);
  for (int i = 0; i < imageHeight; ++i) {
    rowPointers[i] = &imageData[i * imageWidth * 4];
  }

  png_set_rows(png, info, &rowPointers[0]);
  png_write_png(png, info, PNG_TRANSFORM_IDENTITY, nullptr);

  png_destroy_write_struct(&png, &info);
  return true;
}

void invertColors(void) {
  for (int y = 0; y < imageHeight; ++y) {
    for (int x = 0; x < imageWidth; ++x) {
      int idx = (y * imageWidth + x) * 4; // 4 for RGBA
      imageData[idx + 0] = 255 - imageData[idx + 0]; // R
      imageData[idx + 1] = 255 - imageData[idx + 1]; // G
      imageData[idx + 2] = 255 - imageData[idx + 2]; // B
    }
  }
}

void convertToGrayscale(void) {
  for (int y = 0; y < imageHeight; ++y) {
    for (int x = 0; x < imageWidth; ++x) {
      int idx = (y * imageWidth + x) * 4; // 4 for RGBA
      png_byte avg = static_cast<png_byte>((imageData[idx] + imageData[idx + 1] + imageData[idx + 2]) / 3);
      imageData[idx + 0] = avg; // R
      imageData[idx + 1] = avg; // G
      imageData[idx + 2] = avg; // B
    }
  }
}

void renderMainMenu(ImGui::FileBrowser& fileDialog, GLFWwindow* window) {
  // Start of window
  ImGui::SetNextWindowPos(ImVec2(10, 10));
  ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 4));
  ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse);

  // Menu bar
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      // TODO: Implement file menu
      if (ImGui::MenuItem("Open", "Ctrl+O")) {
        // Set the flag to true to open the dialog on the next frame
        openDialog = true;
      }

      if (ImGui::MenuItem("Save", "Ctrl+S")) {
        if (pngLoaded) pngLoaded = !saveImage("../sphere2.png");
      }

      if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {}
          
      ImGui::Separator();
      if (ImGui::MenuItem("Quit", "Ctrl+Q")) {
        glfwSetWindowShouldClose(window, true);
      }

      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  if (openDialog) {
    fileDialog.Open();
    openDialog = false;
  }

  if (!pngLoaded) {
    ImGui::Text("No PNG file loaded");
  }

  // End of window
  ImGui::End();
}

void renderControlPanel(void) {
  ImGui::SetNextWindowPos(ImVec2(10, SCREEN_HEIGHT / 4 + 20));
  ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 4));
  ImGui::Begin("Control Panel", nullptr);

  if (ImGui::Button("Invert Colors")) {
    invertColors();
    updateOpenGLTexture();
  }

  if (ImGui::Button("Convert to Grayscale")) {
    convertToGrayscale();
    updateOpenGLTexture();
  }

  ImGui::End();
}

void renderImageEditorWindow(void) {
  ImGui::SetNextWindowPos(ImVec2(SCREEN_WIDTH / 2, 10));
  ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH / 2 - 10, SCREEN_HEIGHT - 20));
  ImGui::Begin("PNG Editor", nullptr);

  ImVec2 windowPos = ImGui::GetWindowPos();
  ImVec2 windowSize = ImGui::GetWindowSize();
  ImVec2 imagePos = ImVec2(windowPos.x + windowSize.x / 2.0f - imageWidth / 2.0f, windowPos.y + windowSize.y / 2.0f - imageHeight / 2.0f);

  ImGui::SetCursorPos(ImVec2(imagePos.x - windowPos.x, imagePos.y - windowPos.y));
  ImGui::Image(imageTexture, ImVec2(imageWidth, imageHeight));

  ImGui::End();
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
  GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PNGEditor", nullptr, nullptr);
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

  // Create a file browser instance
  ImGui::FileBrowser fileDialog;
  fileDialog.SetTitle("File Browser");
  fileDialog.SetTypeFilters({ ".png" });

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Poll and handle events (inputs, window resize, etc.)
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderMainMenu(fileDialog, window);

    // File Browser
    {
      // Display the file browser
      fileDialog.Display();

      // If a file is selected
      if (fileDialog.HasSelected()) {
        // Load the PNG image
        pngLoaded = loadImage(fileDialog.GetSelected().string());

        // Close the file dialog
        fileDialog.ClearSelected();
      }
    }

    // Control Panel and Image Editor Window
    if (pngLoaded) {
      renderControlPanel();
      if (imageTexture == nullptr) {
        createOpenGLTexture();
      }
      renderImageEditorWindow();
    }

    // Rendering
    ImGui::Render();
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
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