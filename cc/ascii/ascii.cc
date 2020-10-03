#include "ascii.hh"
#include <stdio.h>
#include <math.h>
#define NANOSVG_IMPLEMENTATION	// Expands implementation
#define NANOSVG_CPLUSPLUS
#define NANOSVGRAST_CPLUSPLUS
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg.h"
#include "nanosvgrast.h"

// Convert RGB24 to xterm-256 8-bit value
// For simplicity, assume RGB space is perceptually uniform.
// There are 5 places where one of two outputs needs to be chosen when the
// input is the exact middle:
// - The r/g/b channels and the gray value: the higher value output is chosen.
// - If the gray and color have same distance from the input - color is chosen.
static unsigned char rgb_to_x256(uint8_t r, uint8_t g, uint8_t b)
{
    // Calculate the nearest 0-based color index at 16 .. 231
#   define v2ci(v) (v < 48 ? 0 : v < 115 ? 1 : (v - 35) / 40)
    int ir = v2ci(r), ig = v2ci(g), ib = v2ci(b);   // 0..5 each
#   define color_index() (36 * ir + 6 * ig + ib)  /* 0..215, lazy evaluation */

    // Calculate the nearest 0-based gray index at 232 .. 255
    int average = (r + g + b) / 3;
    int gray_index = average > 238 ? 23 : (average - 3) / 10;  // 0..23

    // Calculate the represented colors back from the index
    static const int i2cv[6] = {0, 0x5f, 0x87, 0xaf, 0xd7, 0xff};
    int cr = i2cv[ir], cg = i2cv[ig], cb = i2cv[ib];  // r/g/b, 0..255 each
    int gv = 8 + 10 * gray_index;  // same value for r/g/b, 0..255

    // Return the one which is nearer to the original input rgb value
#   define dist_square(A,B,C, a,b,c) ((A-a)*(A-a) + (B-b)*(B-b) + (C-c)*(C-c))
    int color_err = dist_square(cr, cg, cb, r, g, b);
    int gray_err  = dist_square(gv, gv, gv, r, g, b);
    return color_err <= gray_err ? 16 + color_index() : 232 + gray_index;
}

void calcWeit(const unsigned char* rgbaImage, unsigned long widthStride, Rect rect, unsigned char& r, unsigned char& g, unsigned char& b) {
  unsigned long long sr = 0;
  unsigned long long sg = 0;
  unsigned long long sb = 0;
  const unsigned char* pixel; 
  for(unsigned long x = rect.x; x < rect.x+rect.w; ++x) { 
    for(unsigned long y = rect.y; y < rect.y+rect.h; ++y) {
      pixel = rgbaImage + (x * 4 + y * widthStride);
      sr += *(pixel++);
      sg += *(pixel++);
      sb += *(pixel++);
    }  
  }
  sr /= rect.w * rect.h;
  sg /= rect.w * rect.h;
  sb /= rect.w * rect.h;
  r = (unsigned char)sr;
  g = (unsigned char)sg;
  b = (unsigned char)sb;
}

void AsciiImage::render(const unsigned char* img, unsigned long width, unsigned long height) {
  const std::string ASCII = " @B#Q80Rg&D$OEN9bd6MHWA%GpPSq5UZ4K3hkfmCFaVeoIs2jtJwzynu1YclT7}{Lxiv[]/\\()|?*r^<>+;~!=\",_-'  ";
  unsigned long w = width;
  unsigned long h = height/2;
  unsigned long rw = width / w;
  unsigned long rh = height / h;
  unsigned char r, g, b;
  unsigned char color;
  unsigned int ind;
  Rect rect;
  float weit;
  unsigned long position;
  for(unsigned long y = 0; y < h; ++y) {
    if ((y+1)*rh > height) {
      break;
    }
    for(unsigned long x = 0; x < w; ++x) {
      if ((x+1)*rw > width) {
	break;
      }
      position = x + y * w;
      rect.x = x * rw;
      rect.y = y * rh;
      rect.w = rw;
      rect.h = rh;
      
      calcWeit(img, width * 4, rect, r, g, b);

      weit = ((float)(r + g + b)) / 3.0f / 255.0f;

      weit = 1-weit;
      weit = weit * weit;
      weit = 1-weit;
      color = rgb_to_x256(r, g, b);
      ind = weit * (ASCII.size() - 1);
      colors[position] = color;
      text[position] = ASCII[ind];
      //printf("\x1B[38;5;%im%c\x1B[0m", color, ASCII[ind]);
    }
    //printf("\n");
  }
  //fflush(stdout);
}

void AsciiImage::load() {
  struct NSVGimage* image;
  float scalex, scaley;
  image = nsvgParseFromFile(path.c_str(), "px", 96);
  if (image == NULL) {
    printf("Error loading image %s\n", path.c_str());
    unload();
    return;
  }
  
  if (respect_width) {
    width = image->width;
    rect.w = width;
  } else {
    width = rect.w;
  }
  
  scalex = ((float)width)/image->width;
  
  if (respect_height) {
    scaley = scalex;
    if (image->width == image->height) {
      height = width;
    } else {
      height = scaley*image->height;
    }
    rect.h = height / 2;
  } else {
    height = rect.h * 2;
    scaley = ((float)height)/image->height;
  }
  text = new unsigned char[rect.w * rect.h];
  colors = new unsigned char[rect.w * rect.h];
  // Rasterize
  unsigned char* img = new unsigned char[width*height*4];
  NSVGrasterizer* rast = nsvgCreateRasterizer();
  nsvgRasterizeStretched(rast, image, 0,0,scalex, scaley, img, width, height, width*4);
  nsvgDeleteRasterizer(rast);
  render(img, width, height);
  delete[] img;
}

void AsciiImage::print() {
  unsigned long position;
  for(unsigned long y = 0; y < rect.h; ++y) {
    for(unsigned long x = 0; x < rect.w; ++x) {
      position = x + y * rect.w;
      printf("\x1B[38;5;%im%c\x1B[0m", colors[position], text[position]);
    }
    printf("\n");
  }
  fflush(stdout);
}
