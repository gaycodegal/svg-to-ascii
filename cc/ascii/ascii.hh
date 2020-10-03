#pragma once
#include <string>
#include <stdlib.h>

struct Rect {
  unsigned long x,y,w,h;
};

class AsciiImage {
 private:
  bool respect_width;
  bool respect_height;
  Rect rect;
  unsigned long width;
  unsigned long height;
  std::string path;
  unsigned char* text = NULL;
  unsigned char* colors = NULL;

  void render(const unsigned char* image, unsigned long width, unsigned long height);
  
 public:
  AsciiImage(std::string path, Rect rect, bool respect_width, bool respect_height): respect_width(respect_width), respect_height(respect_height), rect(rect), width(rect.w), height(rect.h), path(path) {
    load();
  }

  ~AsciiImage() {
    unload();
  }

  void load();
  void print();
  
  void unload() {
    if(text !=NULL){
      delete[] text;
    }
    
    if (colors != NULL) {
      delete[] colors;
    }
    
    text = NULL;
    colors = NULL;
  }
};

static unsigned char rgb_to_x256(uint8_t r, uint8_t g, uint8_t b);
