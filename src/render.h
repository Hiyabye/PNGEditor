#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imfilebrowser.h>
#include "image.h"

/* Constants */
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int MARGIN = 5;

class Renderer {
private:
  ImGui::FileBrowser fileDialog;

public:
  /* Constructor */
  Renderer(void);

  /* Destructor */
  ~Renderer(void);

  /* Methods */
  void renderMainMenu(GLFWwindow* window, Image*& image);
  void renderFileDialog(GLFWwindow* window, Image*& image);
  void renderControlPanel(GLFWwindow* window, Image*& image);
  void renderImageEditorWindow(GLFWwindow* window, Image*& image);
};