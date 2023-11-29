#pragma once

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imfilebrowser.h>
#include "image.h"

/* Constants */
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int MARGIN = 5;

class Renderer {
private:
  /* Private Variables */
  ImGui::FileBrowser fileDialog;
  Image invertIcon;
  Image grayscaleIcon;
  Image blurIcon;
  Image sharpenIcon;
  Image rotateIcon;

public:
  /* Constructor */
  Renderer(void);

  /* Destructor */
  ~Renderer(void);

  /* Methods */
  void renderMainMenu(GLFWwindow* window, ImGuiIO& io, std::unique_ptr<Image>& image);
  void renderFileDialog(GLFWwindow* window, std::unique_ptr<Image>& image);
  void renderControlPanel(GLFWwindow* window, std::unique_ptr<Image>& image);
  void renderImageEditorWindow(GLFWwindow* window, std::unique_ptr<Image>& image);
};