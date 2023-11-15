#include <iostream>
#include <fstream>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "image.h"

/////////////////// IMAGE CONSTRUCTOR ///////////////////

/* Initializes the image class with default values
 *
 * Parameters:
 *  None
 * 
 * Returns:
 *  None
 */
Image::Image(void) {
  this->path = "";
  this->loaded = false;
  this->width = 0;
  this->height = 0;
  this->bitDepth = 0;
  this->colorType = 0;
  this->data = std::vector<png_byte>();
  this->texture = nullptr;
}

/////////////////// IMAGE DESTRUCTOR ////////////////////

/* Deallocates the image class
 * 
 * Parameters:
 *  None
 * 
 * Returns:
 *  None
 */
Image::~Image(void) {
  delete this;
}

/////////////////// IMAGE METHODS ///////////////////////

/* Loads an image from a file into memory
 * 
 * Parameters:
 *  path - The path to the image file
 * 
 * Returns:
 *  None
 */
void Image::load(const std::string path) {
  // Set the path
  this->path = path;

  // Open the file
  FILE* fp = fopen(this->path.c_str(), "rb");
  if (!fp) {
    std::cerr << "Failed to open for reading: " << this->path << std::endl;
    return;
  }
  std::unique_ptr<FILE, decltype(&fclose)> file(fp, fclose);

  // Create a PNG reader
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png) {
    std::cerr << "Failed to create PNG read struct" << std::endl;
    return;
  }

  // Create a PNG info struct
  png_infop info = png_create_info_struct(png);
  if (!info) {
    std::cerr << "Failed to create PNG info struct" << std::endl;
    png_destroy_read_struct(&png, nullptr, nullptr);
    return;
  }

  // Set up error handling
  if (setjmp(png_jmpbuf(png))) {
    std::cerr << "Failed to set PNG jump buffer" << std::endl;
    png_destroy_read_struct(&png, &info, nullptr);
    return;
  }

  // Read the PNG info
  png_init_io(png, fp);
  png_read_info(png, info);
  this->width = png_get_image_width(png, info);
  this->height = png_get_image_height(png, info);
  this->bitDepth = png_get_bit_depth(png, info);
  this->colorType = png_get_color_type(png, info);

  // Convert grayscale images to RGBA
  if (this->colorType == PNG_COLOR_TYPE_GRAY || this->colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
    png_set_gray_to_rgb(png);
  }

  // Convert paletted images to RGBA
  if (this->colorType == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png);
  }

  // Add alpha channel if there is none (RGB -> RGBA)
  if (!(this->colorType & PNG_COLOR_MASK_ALPHA)) {
    png_set_add_alpha(png, 0xFF, PNG_FILLER_AFTER);
  }

  // Ensure 8-bit depth
  if (this->bitDepth == 16) {
    png_set_strip_16(png);
  }

  // Update the PNG info
  png_read_update_info(png, info);

  // Read the PNG image
  png_bytep* rowPointers = nullptr;
  this->data.resize(this->width * this->height * 4);

  // Create a buffer to hold the image data
  rowPointers = new png_bytep[this->height];
  for (int i = 0; i < this->height; ++i) rowPointers[i] = new png_byte[png_get_rowbytes(png, info)];
  png_read_image(png, rowPointers);

  // Flatten the image data
  for (int y = 0; y < this->height; ++y) {
    for (int x = 0; x < this->width; ++x) {
      this->data[4 * (y * this->width + x) + 0] = rowPointers[y][4 * x + 0];
      this->data[4 * (y * this->width + x) + 1] = rowPointers[y][4 * x + 1];
      this->data[4 * (y * this->width + x) + 2] = rowPointers[y][4 * x + 2];
      this->data[4 * (y * this->width + x) + 3] = rowPointers[y][4 * x + 3];
    }
  }

  // Cleanup
  for (int i = 0; i < this->height; ++i) delete[] rowPointers[i];
  delete[] rowPointers;
  png_destroy_read_struct(&png, &info, nullptr);

  this->loaded = true;
}

/* Saves an image from memory to a file
 * 
 * Parameters:
 *  None
 * 
 * Returns:
 *  None
 */
void Image::save(void) {
  // Open the file
  FILE* fp = fopen(this->path.c_str(), "wb");
  if (!fp) {
    std::cerr << "Failed to open for writing: " << this->path << std::endl;
    return;
  }
  std::unique_ptr<FILE, decltype(&fclose)> file(fp, fclose);

  // Create a PNG writer
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png) {
    std::cerr << "Failed to create PNG write struct" << std::endl;
    return;
  }

  // Create a PNG info struct
  png_infop info = png_create_info_struct(png);
  if (!info) {
    std::cerr << "Failed to create PNG info struct" << std::endl;
    png_destroy_write_struct(&png, nullptr);
    return;
  }

  // Set up error handling
  if (setjmp(png_jmpbuf(png))) {
    std::cerr << "Failed to set PNG jump buffer" << std::endl;
    png_destroy_write_struct(&png, &info);
    return;
  }

  // Write the PNG info
  png_init_io(png, fp);
  png_set_IHDR(png, info, this->width, this->height, this->bitDepth, this->colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  // Assume the image data is a contiguous array of RGBA values
  std::vector<png_bytep> rowPointers(this->height);
  for (int i = 0; i < this->height; ++i) rowPointers[i] = &this->data[i * this->width * 4];

  // Write the PNG image
  png_set_rows(png, info, &rowPointers[0]);
  png_write_png(png, info, PNG_TRANSFORM_IDENTITY, nullptr);
  png_write_end(png, nullptr);

  // Cleanup
  png_destroy_write_struct(&png, &info);
  this->loaded = false;
}

/* Creates an OpenGL texture from the image data
 * 
 * Parameters:
 *  None
 * 
 * Returns:
 *  None
 */
void Image::createOpenGLTexture(void) {
  // Generate a texture ID
  GLuint textureID;
  glGenTextures(1, &textureID);
  if (!textureID) {
    std::cerr << "Failed to create OpenGL texture" << std::endl;
    exit(1);
  }

  // Bind the texture
  glBindTexture(GL_TEXTURE_2D, textureID);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "Failed to bind OpenGL texture" << std::endl;
    exit(1);
  }

  // Set the texture parameters
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->data.data());
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "Failed to set OpenGL texture data" << std::endl;
    exit(1);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  this->texture = reinterpret_cast<ImTextureID>(static_cast<intptr_t>(textureID));
}

/* Updates the OpenGL texture with the image data
 * 
 * Parameters:
 *  None
 * 
 * Returns:
 *  None
 */
void Image::updateOpenGLTexture(void) {
  // Bind the texture
  glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)this->texture);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "Failed to bind OpenGL texture" << std::endl;
    return;
  }

  // Update the texture data
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->data.data());
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "Failed to update OpenGL texture data" << std::endl;
    return;
  }
}

/* Invert the colors of the image
 * 
 * Parameters:
 *  None
 * 
 * Returns:
 *  None
 */
void Image::invert(void) {
  for (int y = 0; y < this->height; ++y) {
    for (int x = 0; x < this->width; ++x) {
      int idx = 4 * (y * this->width + x); // 4 channels (RGBA)
      this->data[idx + 0] = 255 - this->data[idx + 0]; // R
      this->data[idx + 1] = 255 - this->data[idx + 1]; // G
      this->data[idx + 2] = 255 - this->data[idx + 2]; // B
    }
  }
  this->updateOpenGLTexture();
}

/* Grayscale the image
 * 
 * Parameters:
 *  None
 * 
 * Returns:
 *  None
 */
void Image::grayscale(void) {
  for (int y = 0; y < this->height; ++y) {
    for (int x = 0; x < this->width; ++x) {
      int idx = 4 * (y * this->width + x); // 4 channels (RGBA)
      png_byte avg = static_cast<png_byte>((this->data[idx + 0] + this->data[idx + 1] + this->data[idx + 2]) / 3);
      this->data[idx + 0] = avg; // R
      this->data[idx + 1] = avg; // G
      this->data[idx + 2] = avg; // B
    }
  }
  this->updateOpenGLTexture();
}

/////////////////// IMAGE GETTERS ///////////////////

std::string Image::getPath(void) const { return this->path; }
bool Image::isLoaded(void) const { return this->loaded; }
int Image::getWidth(void) const { return this->width; }
int Image::getHeight(void) const { return this->height; }
int Image::getBitDepth(void) const { return this->bitDepth; }
int Image::getColorType(void) const { return this->colorType; }
std::vector<png_byte> Image::getData(void) const { return this->data; }
ImTextureID Image::getTexture(void) const { return this->texture; }

/////////////////// IMAGE SETTERS ///////////////////

void Image::setPath(std::string path) { this->path = path; }
void Image::setLoaded(bool loaded) { this->loaded = loaded; }
void Image::setWidth(int width) { this->width = width; }
void Image::setHeight(int height) { this->height = height; }
void Image::setBitDepth(int bitDepth) { this->bitDepth = bitDepth; }
void Image::setColorType(int colorType) { this->colorType = colorType; }
void Image::setData(std::vector<png_byte> data) { this->data = data; }
void Image::setTexture(ImTextureID texture) { this->texture = texture; }