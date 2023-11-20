#pragma once

#include <string>
#include <vector>
#include <png.h>
#include <imgui.h>

class Image {
private:
  /* Private Variables */
  std::string path;
  bool loaded;
  int width;
  int height;
  int bitDepth;
  int colorType;
  std::vector<png_byte> originalData;
  std::vector<png_byte> data;
  ImTextureID texture;
  bool _invert;
  bool _grayscale;
  bool _blur;
  bool _sharpen;

public:
  /* Public Variables */
  float red;
  float green;
  float blue;
  int rotateAngle;

  /* Constructor */
  Image(void);

  /* Destructor */
  ~Image(void);

  /* Methods */
  void load(const std::string path);
  void save(void);
  void createOpenGLTexture(void);
  void updateOpenGLTexture(void);
  void applyKernel(const float kernel[][3]);
  void reset(void);
  void invert(void);
  void grayscale(void);
  void blur(void);
  void sharpen(void);
  void rgb(void);
  void rotate(void);

  /* Getters */
  std::string getPath(void) const;
  bool isLoaded(void) const;
  int getWidth(void) const;
  int getHeight(void) const;
  int getBitDepth(void) const;
  int getColorType(void) const;
  std::vector<png_byte> getData(void) const;
  ImTextureID getTexture(void) const;
  bool isInvert(void) const;
  bool isGrayscale(void) const;
  bool isBlur(void) const;
  bool isSharpen(void) const;

  /* Setters */
  void setPath(std::string path);
  void setLoaded(bool loaded);
  void setWidth(int width);
  void setHeight(int height);
  void setBitDepth(int bitDepth);
  void setColorType(int colorType);
  void setData(std::vector<png_byte> data);
  void setTexture(ImTextureID texture);
  void setInvert(bool invert);
  void setGrayscale(bool grayscale);
  void setBlur(bool blur);
  void setSharpen(bool sharpen);
};