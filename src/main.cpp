#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "png.h"

static void glfw_error_callback(int error, const char* description) {
  std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

bool loadPNGImage(const char* filename, ImVec2& imageSize, ImTextureID& texture) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    std::cerr << "Failed to open PNG file: " << filename << std::endl;
    return false;
  }

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png) {
    std::cerr << "Failed to create PNG read struct" << std::endl;
    fclose(file);
    return false;
  }

  png_infop info = png_create_info_struct(png);
  if (!info) {
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
  int bitDepth = png_get_bit_depth(png, info);
  int colorType = png_get_color_type(png, info);

  // Handle different color types
  if (colorType == PNG_COLOR_TYPE_RGB) {
    // RGB image
    // You can add code to display the image here
    // Create a buffer to hold the image data
    size_t row_bytes = png_get_rowbytes(png, info);
    png_bytep* row_pointers = new png_bytep[height];
    for (int i = 0; i < height; i++) row_pointers[i] = new png_byte[row_bytes];

    png_read_image(png, row_pointers);

    // Create an ImGui texture from the image data
    texture = ImGui::GetIO().Fonts->TexID;
    imageSize = ImVec2(static_cast<float>(width), static_cast<float>(height));

    // Clean up
    for (int i = 0; i < height; ++i) delete[] row_pointers[i];
    delete[] row_pointers;
  } else if (colorType == PNG_COLOR_TYPE_RGBA) {
    // RGBA image
    // You can add code to display the image here
    // Create a buffer to hold the image data
    size_t row_bytes = png_get_rowbytes(png, info);
    png_bytep* row_pointers = new png_bytep[height];
    for (int i = 0; i < height; i++) row_pointers[i] = new png_byte[row_bytes];

    png_read_image(png, row_pointers);

    // Create an ImGui texture from the image data
    texture = ImGui::GetIO().Fonts->TexID;
    imageSize = ImVec2(static_cast<float>(width), static_cast<float>(height));

    // Clean up
    for (int i = 0; i < height; ++i) delete[] row_pointers[i];
    delete[] row_pointers;
  } else {
    // Unsupported color type
    std::cerr << "Unsupported PNG color type: " << colorType << std::endl;
  }

  // Clean up
  png_destroy_read_struct(&png, &info, NULL);
  fclose(file);

  return true;
}

int main(int argc, char *argv[]) {
  // Initialize GLFW
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) return 1;

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
  if (window == nullptr) return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Initialize OpenGL loader
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

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
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

      ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
      ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);

      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

      if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
      ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
      ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me"))
        show_another_window = false;
      ImGui::End();
    }

    // 4. Show a PNG image
    {
      static ImTextureID pngTexture;
      static ImVec2 imageSize;

      if (loadPNGImage("../sample.png", imageSize, pngTexture)) {
        // Display the loaded PNG image within an ImGui window
        ImGui::Begin("Image Window");
        ImGui::Image(pngTexture, imageSize);
        ImGui::End();
      }
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
