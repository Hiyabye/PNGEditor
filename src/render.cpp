#include "render.h"

/////////////////// RENDERER CONSTRUCTOR ///////////////////

// @brief: Initializes the renderer class with default values
Renderer::Renderer(void) {
  // Initialize file dialog
  this->fileDialog.SetTitle("Select PNG file");
  this->fileDialog.SetTypeFilters({ ".png" });

  // Initialize icons
  this->invertIcon.load("assets/invert.png");
  this->invertIcon.createOpenGLTexture();
  this->grayscaleIcon.load("assets/grayscale.png");
  this->grayscaleIcon.createOpenGLTexture();
  this->blurIcon.load("assets/blur.png");
  this->blurIcon.createOpenGLTexture();
  this->sharpenIcon.load("assets/sharpen.png");
  this->sharpenIcon.createOpenGLTexture();
  this->rotateIcon.load("assets/rotate.png");
  this->rotateIcon.createOpenGLTexture();
}

/////////////////// RENDERER DESTRUCTOR ////////////////////

// @brief: Deallocates the renderer class
Renderer::~Renderer(void) {
  // Deallocate file dialog
  this->fileDialog.ClearSelected();

  // Deallocate icons
  this->invertIcon.~Image();
  this->grayscaleIcon.~Image();
  this->blurIcon.~Image();
  this->sharpenIcon.~Image();
  this->rotateIcon.~Image();

  // Deallocate renderer
  delete this;
}

/////////////////// RENDERER METHODS //////////////////////

// @brief: Renders the main menu
// @param `window`: The GLFW window
// @param `image`: The image to render
void Renderer::renderMainMenu(GLFWwindow* window, ImGuiIO& io, std::unique_ptr<Image>& image) {
  ImGui::SetNextWindowPos(ImVec2(MARGIN, MARGIN), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 4), ImGuiCond_Once);
  ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

  // Menu bar variables
  bool open = false;
  bool save = false;
  bool saveAs = false;
  bool quit = false;

  // Render the menu bar
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open", "Ctrl+O")) open = true;
      if (ImGui::MenuItem("Save", "Ctrl+S")) save = true;
      if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) saveAs = true;
      ImGui::Separator();
      if (ImGui::MenuItem("Quit", "Ctrl+Q")) quit = true;
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  // Keyboard shortcuts
  bool ctrl = io.KeyCtrl;
  if (ctrl && io.KeysDown[ImGuiKey_O]) open = true;
  if (ctrl && io.KeysDown[ImGuiKey_S]) save = true;
  if (ctrl && io.KeysDown[ImGuiKey_S] && io.KeyShift) saveAs = true;
  if (ctrl && io.KeysDown[ImGuiKey_Q]) quit = true;

  // Process menu bar actions
  if (open) {
    if (image->isLoaded()) {
      // factory reset the image; the reset function is not used in this code
    }
    this->fileDialog.Open();
  }

  if (save) {
    if (image->isLoaded()) image->save();
  }

  if (saveAs) {
    // TODO - Implement this function
  }

  if (quit) glfwSetWindowShouldClose(window, true);

  // Render the main menu
  if (!image->isLoaded()) {
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize("No PNG file loaded").x / 2);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2 - ImGui::CalcTextSize("No PNG file loaded").y / 2);
    ImGui::Text("No PNG file loaded");
  }

  ImGui::End();
}

// @brief: Renders the file dialog
// @param `window`: The GLFW window
// @param `image`: The image to render
void Renderer::renderFileDialog(GLFWwindow* window, std::unique_ptr<Image>& image) {
  this->fileDialog.Display();
  if (this->fileDialog.HasSelected()) {
    image->load(this->fileDialog.GetSelected().string());
    this->fileDialog.ClearSelected();
    this->fileDialog.Close();
  }
}

// @brief: Renders the control panel
// @param `window`: The GLFW window
// @param `image`: The image to render
void Renderer::renderControlPanel(GLFWwindow* window, std::unique_ptr<Image>& image) {
  ImGui::SetNextWindowPos(ImVec2(MARGIN, SCREEN_HEIGHT / 4 + MARGIN * 2), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH / 6, 3 * SCREEN_HEIGHT / 4 - MARGIN * 3), ImGuiCond_Once);
  ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

  // Image editing functions
  bool update = false;
  if (ImGui::ImageButton(this->invertIcon.getTexture(), ImVec2(32, 32))) { image->setInvert(!image->isInvert()); update = true; }
  if (ImGui::ImageButton(this->grayscaleIcon.getTexture(), ImVec2(32, 32))) { image->setGrayscale(!image->isGrayscale()); update = true; }
  if (ImGui::ImageButton(this->blurIcon.getTexture(), ImVec2(32, 32))) { image->setBlur(!image->isBlur()); update = true; }
  if (ImGui::ImageButton(this->sharpenIcon.getTexture(), ImVec2(32, 32))) { image->setSharpen(!image->isSharpen()); update = true; }
  if (ImGui::SliderFloat("Red", &image->red, 0.0f, 1.0f)) update = true;
  if (ImGui::SliderFloat("Green", &image->green, 0.0f, 1.0f)) update = true;
  if (ImGui::SliderFloat("Blue", &image->blue, 0.0f, 1.0f)) update = true;

  static bool rotate = false;
  if (ImGui::ImageButton(this->rotateIcon.getTexture(), ImVec2(32, 32))) rotate = !rotate;
  if (rotate && ImGui::SliderInt("Angle", &image->rotateAngle, -180, 180)) update = true;

  // Update the image
  if (update) {
    image->reset();
    if (image->isInvert()) image->invert();
    if (image->isGrayscale()) image->grayscale();
    if (image->isBlur()) image->blur();
    if (image->isSharpen()) image->sharpen();
    image->rgb();
    image->rotate();
    image->updateOpenGLTexture();
  }

  ImGui::End();
}

// @brief: Renders the image editor window
// @param `window`: The GLFW window
// @param `image`: The image to render
void Renderer::renderImageEditorWindow(GLFWwindow* window, std::unique_ptr<Image>& image) {
  ImGui::SetNextWindowPos(ImVec2(SCREEN_WIDTH / 6 + MARGIN * 2, MARGIN), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(5 * SCREEN_WIDTH / 6 - MARGIN * 3, SCREEN_HEIGHT - MARGIN * 2), ImGuiCond_Once);
  ImGui::Begin(image->getPath().c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

  ImVec2 windowPos = ImGui::GetWindowPos();
  ImVec2 windowSize = ImGui::GetWindowSize();
  ImVec2 imagePos = ImVec2(windowPos.x + windowSize.x / 2.0f - image->getWidth() / 2.0f, windowPos.y + windowSize.y / 2.0f - image->getHeight() / 2.0f);

  ImGui::SetCursorPos(ImVec2(imagePos.x - windowPos.x, imagePos.y - windowPos.y));
  ImGui::Image(image->getTexture(), ImVec2(image->getWidth(), image->getHeight()));

  ImGui::End();
}