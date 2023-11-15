#pragma once

#include <string>
#include <vector>
#include <png.h>

class Image {
private:
  std::string path;
  bool loaded;
  int width;
  int height;
  int bitDepth;
  int colorType;
  std::vector<png_byte> data;
  ImTextureID texture;

public:
  /* Constructor */
  Image(void);

  /* Destructor */
  ~Image(void);

  /* Methods */
  void load(const std::string path);
  void save(void);
  void createOpenGLTexture(void);
  void updateOpenGLTexture(void);
  void invert(void);  
  void grayscale(void);

  /* Getters */
  std::string getPath(void) const;
  bool isLoaded(void) const;
  int getWidth(void) const;
  int getHeight(void) const;
  int getBitDepth(void) const;
  int getColorType(void) const;
  std::vector<png_byte> getData(void) const;
  ImTextureID getTexture(void) const;

  /* Setters */
  void setPath(std::string path);
  void setLoaded(bool loaded);
  void setWidth(int width);
  void setHeight(int height);
  void setBitDepth(int bitDepth);
  void setColorType(int colorType);
  void setData(std::vector<png_byte> data);
  void setTexture(ImTextureID texture);
};