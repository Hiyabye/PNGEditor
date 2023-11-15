#include "render.h"

/////////////////// RENDERER CONSTRUCTOR ///////////////////

/* Initializes the renderer class with default values
 *
 * Parameters:
 *  None
 * 
 * Returns:
 *  None
 */
Renderer::Renderer(void) {
  // Initialize file dialog
  this->fileDialog.SetTitle("Select PNG file");
  this->fileDialog.SetTypeFilters({ ".png" });
}

/////////////////// RENDERER DESTRUCTOR ////////////////////

/* Deallocates the renderer class
 * 
 * Parameters:
 *  None
 * 
 * Returns:
 *  None
 */
Renderer::~Renderer(void) {
  delete this;
}

/////////////////// RENDERER METHODS //////////////////////

/* Renders the main menu
 * 
 * Parameters:
 *  window - The GLFW window
 *  image - The image to render
 * 
 * Returns:
 *  None
 */
void Renderer::renderMainMenu(GLFWwindow* window, Image*& image) {
  ImGui::SetNextWindowPos(ImVec2(MARGIN, MARGIN), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 4), ImGuiCond_Once);
  ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

  // Render the menu bar
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open", "Ctrl+O")) this->fileDialog.Open();
      if (ImGui::MenuItem("Save", "Ctrl+S")) if (image->isLoaded()) image->save();
      if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {} // TODO - Implement this function
      ImGui::Separator();
      if (ImGui::MenuItem("Quit", "Ctrl+Q")) glfwSetWindowShouldClose(window, true);
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  if (!image->isLoaded()) ImGui::Text("No PNG file loaded");

  ImGui::End();
}

/* Renders the file dialog
 * 
 * Parameters:
 *  window - The GLFW window
 *  image - The image to render
 * 
 * Returns:
 *  None
 */
void Renderer::renderFileDialog(GLFWwindow* window, Image*& image) {
  this->fileDialog.Display();
  if (this->fileDialog.HasSelected()) {
    image->load(this->fileDialog.GetSelected().string());
    this->fileDialog.ClearSelected();
    this->fileDialog.Close();
  }
}

/* Renders the control panel
 * 
 * Parameters:
 *  window - The GLFW window
 *  image - The image to render
 * 
 * Returns:
 *  None
 */
void Renderer::renderControlPanel(GLFWwindow* window, Image*& image) {
  ImGui::SetNextWindowPos(ImVec2(MARGIN, SCREEN_HEIGHT / 4 + MARGIN * 2), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH / 6, 3 * SCREEN_HEIGHT / 4 - MARGIN * 3), ImGuiCond_Once);
  ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

  // Image editing functions
  bool update = false;
  if (ImGui::Button("Invert Colors")) { image->setInvert(!image->isInvert()); update = true; }
  if (ImGui::Button("Convert to Grayscale")) { image->setGrayscale(!image->isGrayscale()); update = true; }
  if (ImGui::Button("Blur")) { image->setBlur(!image->isBlur()); update = true; }
  if (ImGui::Button("Sharpen")) { image->setSharpen(!image->isSharpen()); update = true; }
  if (ImGui::SliderFloat("Red", &image->red, 0.0f, 1.0f) ||
      ImGui::SliderFloat("Green", &image->green, 0.0f, 1.0f) ||
      ImGui::SliderFloat("Blue", &image->blue, 0.0f, 1.0f)) update = true;

  // Update the image
  if (update) {
    image->reset();
    if (image->isInvert()) image->invert();
    if (image->isGrayscale()) image->grayscale();
    if (image->isBlur()) image->blur();
    if (image->isSharpen()) image->sharpen();
    image->rgb();
    image->updateOpenGLTexture();
  }

  ImGui::End();
}

/* Renders the image editor window
 * 
 * Parameters:
 *  window - The GLFW window
 *  image - The image to render
 * 
 * Returns:
 *  None
 */
void Renderer::renderImageEditorWindow(GLFWwindow* window, Image*& image) {
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