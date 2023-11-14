#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imfilebrowser.h>
#include <png.h>

// Constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int MARGIN = 5;

// Global variables
bool openDialog = false;
bool pngLoaded = false;
std::string filename;
int imageWidth = 0;
int imageHeight = 0;
int bitDepth = 0;
int colorType = 0;
std::vector<png_byte> imageData;
ImTextureID imageTexture;

// Create an OpenGL texture from the image data
ImTextureID createOpenGLTexture(void) {
  GLuint textureID;
  
  // Generate a texture ID
  glGenTextures(1, &textureID);
  if (!textureID) {
    std::cerr << "Failed to create OpenGL texture" << std::endl;
    return 0;
  }

  // Bind the texture
  glBindTexture(GL_TEXTURE_2D, textureID);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "Failed to bind OpenGL texture" << std::endl;
    return 0;
  }

  // Set the texture parameters
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "Failed to set OpenGL texture data" << std::endl;
    return 0;
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return reinterpret_cast<ImTextureID>(static_cast<intptr_t>(textureID));
}

// Update the OpenGL texture with the image data
void updateOpenGLTexture(void) {
  // Bind the texture
  glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)imageTexture);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "Failed to bind OpenGL texture" << std::endl;
    return;
  }

  // Update the texture data
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "Failed to update OpenGL texture data" << std::endl;
    return;
  }
}

// Load a PNG image
bool loadImage(void) {
  // Open the PNG file
  FILE* fp = fopen(filename.c_str(), "rb");
  if (!fp) {
    std::cerr << "Failed to open for reading: " << filename << std::endl;
    return false;
  }
  std::unique_ptr<FILE, decltype(&fclose)> file(fp, fclose);

  // Create a PNG reader
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

  // Set up error handling
  if (setjmp(png_jmpbuf(png))) {
    std::cerr << "Failed to set PNG jump buffer" << std::endl;
    png_destroy_read_struct(&png, &info, nullptr);
    return false;
  }

  // Read the PNG info
  png_init_io(png, fp);
  png_read_info(png, info);
  imageWidth = png_get_image_width(png, info);
  imageHeight = png_get_image_height(png, info);
  bitDepth = png_get_bit_depth(png, info);
  colorType = png_get_color_type(png, info);

  // Convert grayscale images to RGBA
  if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
    png_set_gray_to_rgb(png);
  }

  // Convert paletted images to RGBA
  if (colorType == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png);
  }

  // Add alpha channel if there is none (RGB -> RGBA)
  if (!(colorType & PNG_COLOR_MASK_ALPHA)) {
    png_set_add_alpha(png, 0xFF, PNG_FILLER_AFTER);
  }

  // Ensure 8-bit depth
  if (bitDepth == 16) {
    png_set_strip_16(png);
  }

  // Update the PNG info
  png_read_update_info(png, info);

  // Read the PNG image
  png_bytep* rowPointers = nullptr;
  imageData.resize(imageWidth * imageHeight * 4);

  // Create a buffer to hold the image data
  rowPointers = new png_bytep[imageHeight];
  for (int i = 0; i < imageHeight; ++i) rowPointers[i] = new png_byte[png_get_rowbytes(png, info)];
  png_read_image(png, rowPointers);

  // Flatten the image data
  for (int y = 0; y < imageHeight; ++y) {
    for (int x = 0; x < imageWidth; ++x) {
      imageData[4 * (y * imageWidth + x) + 0] = rowPointers[y][4 * x + 0];
      imageData[4 * (y * imageWidth + x) + 1] = rowPointers[y][4 * x + 1];
      imageData[4 * (y * imageWidth + x) + 2] = rowPointers[y][4 * x + 2];
      imageData[4 * (y * imageWidth + x) + 3] = rowPointers[y][4 * x + 3];
    }
  }

  // Cleanup
  for (int i = 0; i < imageHeight; ++i) delete[] rowPointers[i];
  delete[] rowPointers;
  png_destroy_read_struct(&png, &info, nullptr);

  return true;
}

// Save a PNG image
bool saveImage(void) {
  // Open the PNG file
  FILE* fp = fopen(filename.c_str(), "wb");
  if (!fp) {
    std::cerr << "Failed to open for writing: " << filename << std::endl;
    return false;
  }
  std::unique_ptr<FILE, decltype(&fclose)> file(fp, fclose);

  // Create a PNG writer
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

  // Set up error handling
  if (setjmp(png_jmpbuf(png))) {
    std::cerr << "Failed to set PNG jump buffer" << std::endl;
    png_destroy_write_struct(&png, &info);
    return false;
  }

  // Write the PNG info
  png_init_io(png, fp);
  png_set_IHDR(png, info, imageWidth, imageHeight, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  // Assume the image data is a contiguous array of RGBA values
  std::vector<png_bytep> rowPointers(imageHeight);
  for (int i = 0; i < imageHeight; ++i) rowPointers[i] = &imageData[i * imageWidth * 4];

  // Write the PNG image
  png_set_rows(png, info, &rowPointers[0]);
  png_write_png(png, info, PNG_TRANSFORM_IDENTITY, nullptr);
  png_write_end(png, nullptr);

  // Cleanup
  png_destroy_write_struct(&png, &info);

  return true;
}

// Invert the colors of the image
// R = 255 - R, G = 255 - G, B = 255 - B
void invertColors(void) {
  for (int y = 0; y < imageHeight; ++y) {
    for (int x = 0; x < imageWidth; ++x) {
      int idx = 4 * (y * imageWidth + x); // 4 channels (RGBA)
      imageData[idx + 0] = 255 - imageData[idx + 0]; // R
      imageData[idx + 1] = 255 - imageData[idx + 1]; // G
      imageData[idx + 2] = 255 - imageData[idx + 2]; // B
    }
  }
  updateOpenGLTexture();
}

// Convert the image to grayscale
// R = G = B = (R + G + B) / 3
void convertToGrayscale(void) {
  for (int y = 0; y < imageHeight; ++y) {
    for (int x = 0; x < imageWidth; ++x) {
      int idx = 4 * (y * imageWidth + x); // 4 channels (RGBA)
      png_byte avg = static_cast<png_byte>((imageData[idx + 0] + imageData[idx + 1] + imageData[idx + 2]) / 3);
      imageData[idx + 0] = avg; // R
      imageData[idx + 1] = avg; // G
      imageData[idx + 2] = avg; // B
    }
  }
  updateOpenGLTexture();
}

// Render the main menu
void renderMainMenu(GLFWwindow* window, ImGui::FileBrowser& fileDialog) {
  ImGui::SetNextWindowPos(ImVec2(MARGIN, MARGIN), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 4), ImGuiCond_Once);
  ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open", "Ctrl+O")) fileDialog.Open();
      if (ImGui::MenuItem("Save", "Ctrl+S")) if (pngLoaded) pngLoaded = !saveImage();
      if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {} // TODO - Implement this function
      ImGui::Separator();
      if (ImGui::MenuItem("Quit", "Ctrl+Q")) glfwSetWindowShouldClose(window, true);
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  if (!pngLoaded) ImGui::Text("No PNG file loaded");

  ImGui::End();
}

// Render the file dialog
void renderFileDialog(ImGui::FileBrowser& fileDialog) {
  fileDialog.Display();
  if (fileDialog.HasSelected()) {
    filename = fileDialog.GetSelected().string();
    fileDialog.ClearSelected();
    fileDialog.Close();
    pngLoaded = loadImage();
  }
}

// Render the control panel
void renderControlPanel(void) {
  ImGui::SetNextWindowPos(ImVec2(MARGIN, SCREEN_HEIGHT / 4 + MARGIN * 2), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH / 6, 3 * SCREEN_HEIGHT / 4 - MARGIN * 3), ImGuiCond_Once);
  ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

  // TODO - Implement more image editing functions
  if (ImGui::Button("Invert Colors")) invertColors();
  if (ImGui::Button("Convert to Grayscale")) convertToGrayscale();

  ImGui::End();
}

// Render the image editor window
void renderImageEditorWindow(void) {
  ImGui::SetNextWindowPos(ImVec2(SCREEN_WIDTH / 6 + MARGIN * 2, MARGIN), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(5 * SCREEN_WIDTH / 6 - MARGIN * 3, SCREEN_HEIGHT - MARGIN * 2), ImGuiCond_Once);
  ImGui::Begin(filename.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

  ImVec2 windowPos = ImGui::GetWindowPos();
  ImVec2 windowSize = ImGui::GetWindowSize();
  ImVec2 imagePos = ImVec2(windowPos.x + windowSize.x / 2.0f - imageWidth / 2.0f, windowPos.y + windowSize.y / 2.0f - imageHeight / 2.0f);

  ImGui::SetCursorPos(ImVec2(imagePos.x - windowPos.x, imagePos.y - windowPos.y));
  ImGui::Image(imageTexture, ImVec2(imageWidth, imageHeight));

  ImGui::End();
}

// Main function
int main(int argc, char* argv[]) {
  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return 1;
  }
  
  // Define GLFW window properties
#if defined(IMGUI_IMPL_OPENGL_ES2) // OpenGL ES 2.0
  // GL ES 2.0 + GLSL 100
  const char* glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__) // MacOS
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else // OpenGL 3.0
  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

  // Create GLFW window
  GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PNG Editor", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    glfwTerminate();
    return 1;
  }

  // Initialize Dear ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos; // Enable mouse cursor
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls
  ImGui::StyleColorsDark();

  // Initialize ImGui GLFW and OpenGL3
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Initialize file dialog
  ImGui::FileBrowser fileDialog;
  fileDialog.SetTitle("Open PNG Image");
  fileDialog.SetTypeFilters({ ".png" });

  // Our state
  ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Poll and handle events (inputs, window resize, etc.)
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render the main menu
    renderMainMenu(window, fileDialog);

    // Render the file dialog
    renderFileDialog(fileDialog);

    // Render the contol panel
    if (pngLoaded) renderControlPanel();

    // Create an OpenGL texture from the image data
    if (pngLoaded && !imageTexture) imageTexture = createOpenGLTexture();

    // Render the image editor window
    if (pngLoaded) renderImageEditorWindow();

    // Rendering
    ImGui::Render();
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
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