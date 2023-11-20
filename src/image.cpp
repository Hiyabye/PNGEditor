#include <iostream>
#include <fstream>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "image.h"

/////////////////// IMAGE CONSTRUCTOR ///////////////////

// @brief: Initializes the image class with default values
Image::Image(void) {
  this->path = "";
  this->loaded = false;
  this->width = 0;
  this->height = 0;
  this->bitDepth = 0;
  this->colorType = 0;
  this->originalData = std::vector<png_byte>();
  this->data = std::vector<png_byte>();
  this->texture = nullptr;
  this->_invert = false;
  this->_grayscale = false;
  this->_blur = false;
  this->_sharpen = false;
  this->red = 1.0f;
  this->green = 1.0f;
  this->blue = 1.0f;
}

/////////////////// IMAGE DESTRUCTOR ////////////////////

// @brief: Deallocates the image class
Image::~Image(void) {
  // Deallocate image
  delete this;
}

/////////////////// IMAGE METHODS ///////////////////////

// @brief: Loads an image from a file into memory
// @param `path`: The path to the image file
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

  // Save the original image data
  this->originalData = this->data;

  // Cleanup
  for (int i = 0; i < this->height; ++i) delete[] rowPointers[i];
  delete[] rowPointers;
  png_destroy_read_struct(&png, &info, nullptr);

  this->loaded = true;
}

// @brief: Saves an image from memory to a file
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

// @brief: Creates an OpenGL texture from the image data
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

// @brief: Updates the OpenGL texture with the image data
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

// @brief: Applies a kernel to the image
// @param `kernel`: The kernel to apply
void Image::applyKernel(const float kernel[][3]) {
  std::vector<png_byte> tmp = this->data;

  // Default kernel size is 3x3
  // The average of the kernel should be 1 to maintain the same brightness
  // TODO: Allow for different kernel sizes
  for (int y = 0; y < this->height - 2; ++y) {
    for (int x = 0; x < this->width - 2; ++x) {
      int idx = 4 * ((y + 1) * this->width + (x + 1)); // 4 channels (RGBA)

      // Apply the kernel
      float red_avg = 0, green_avg = 0, blue_avg = 0;
      for (int ky = 0; ky < 3; ++ky) {
        for (int kx = 0; kx < 3; ++kx) {
          int kidx = 4 * ((y + ky) * this->width + (x + kx)); // 4 channels (RGBA)
          red_avg += static_cast<float>(tmp[kidx + 0]) * kernel[ky][kx] / 9; // R
          green_avg += static_cast<float>(tmp[kidx + 1]) * kernel[ky][kx] / 9; // G
          blue_avg += static_cast<float>(tmp[kidx + 2]) * kernel[ky][kx] / 9; // B
        }
      }

      // Clamp the values to 0-255
      if (red_avg < 0) red_avg = 0;
      if (red_avg > 255) red_avg = 255;
      if (green_avg < 0) green_avg = 0;
      if (green_avg > 255) green_avg = 255;
      if (blue_avg < 0) blue_avg = 0;
      if (blue_avg > 255) blue_avg = 255;

      // Set the new pixel values
      this->data[idx + 0] = static_cast<png_byte>(red_avg); // R
      this->data[idx + 1] = static_cast<png_byte>(green_avg); // G
      this->data[idx + 2] = static_cast<png_byte>(blue_avg); // B
    }
  }
}

// @brief: Resets the image to its original state
void Image::reset(void) {
  this->data = this->originalData;
}

// @brief: Inverts the colors of the image
void Image::invert(void) {
  for (int y = 0; y < this->height; ++y) {
    for (int x = 0; x < this->width; ++x) {
      int idx = 4 * (y * this->width + x); // 4 channels (RGBA)
      this->data[idx + 0] = 255 - this->data[idx + 0]; // R
      this->data[idx + 1] = 255 - this->data[idx + 1]; // G
      this->data[idx + 2] = 255 - this->data[idx + 2]; // B
    }
  }
}

// @brief: Grayscales the image
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
}

// @brief: Blurs the image
void Image::blur(void) {
  const float kernel[3][3] = {
    { 1, 1, 1 },
    { 1, 1, 1 },
    { 1, 1, 1 }
  };
  this->applyKernel(kernel);
}

// @brief: Sharpens the image
void Image::sharpen(void) {
  const float kernel[3][3] = {
    { 0.25, 0.25, 0.25 },
    { 0.25, 7, 0.25 },
    { 0.25, 0.25, 0.25 }
  };
  this->applyKernel(kernel);
}

// @brief: Sets the image's RGB values to the given values
void Image::rgb(void) {
  for (int y = 0; y < this->height; ++y) {
    for (int x = 0; x < this->width; ++x) {
      int idx = 4 * (y * this->width + x); // 4 channels (RGBA)
      this->data[idx + 0] = static_cast<png_byte>(this->data[idx + 0] * red); // R
      this->data[idx + 1] = static_cast<png_byte>(this->data[idx + 1] * green); // G
      this->data[idx + 2] = static_cast<png_byte>(this->data[idx + 2] * blue); // B
    }
  }
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
bool Image::isInvert(void) const { return this->_invert; }
bool Image::isGrayscale(void) const { return this->_grayscale; }
bool Image::isBlur(void) const { return this->_blur; }
bool Image::isSharpen(void) const { return this->_sharpen; }

/////////////////// IMAGE SETTERS ///////////////////

void Image::setPath(std::string path) { this->path = path; }
void Image::setLoaded(bool loaded) { this->loaded = loaded; }
void Image::setWidth(int width) { this->width = width; }
void Image::setHeight(int height) { this->height = height; }
void Image::setBitDepth(int bitDepth) { this->bitDepth = bitDepth; }
void Image::setColorType(int colorType) { this->colorType = colorType; }
void Image::setData(std::vector<png_byte> data) { this->data = data; }
void Image::setTexture(ImTextureID texture) { this->texture = texture; }
void Image::setInvert(bool invert) { this->_invert = invert; }
void Image::setGrayscale(bool grayscale) { this->_grayscale = grayscale; }
void Image::setBlur(bool blur) { this->_blur = blur; }
void Image::setSharpen(bool sharpen) { this->_sharpen = sharpen; }