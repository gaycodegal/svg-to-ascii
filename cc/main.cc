#include <stdio.h>
#include <string.h>
#include <string>
#include <math.h>
#define NANOSVG_IMPLEMENTATION	// Expands implementation
#define NANOSVG_CPLUSPLUS
#define NANOSVGRAST_CPLUSPLUS
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg.h"
#include "nanosvgrast.h"

struct Rect {
  unsigned long x,y,w,h;
};

// Convert RGB24 to xterm-256 8-bit value
// For simplicity, assume RGB space is perceptually uniform.
// There are 5 places where one of two outputs needs to be chosen when the
// input is the exact middle:
// - The r/g/b channels and the gray value: the higher value output is chosen.
// - If the gray and color have same distance from the input - color is chosen.
static int rgb_to_x256(uint8_t r, uint8_t g, uint8_t b)
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

void calcWeit(unsigned char* rgbaImage, unsigned long widthStride, Rect rect, unsigned char& r, unsigned char& g, unsigned char& b) {
  unsigned long long sr = 0;
  unsigned long long sg = 0;
  unsigned long long sb = 0;
  unsigned char* pixel; 
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

void printArt(unsigned char* img, unsigned long width, unsigned long height, unsigned long w, unsigned long h2) {
  const std::string ASCII = " @B#Q80Rg&D$OEN9bd6MHWA%GpPSq5UZ4K3hXkfmCFaVeoIs2jtJwzynu1YclT7}{Lxiv[]/\\()|?*r^<>+;~!=\",_-'  ";
  unsigned long h = h2 / 2;
  unsigned long rw = width / w;
  unsigned long rh = height / h;
  unsigned char r, g, b;
  unsigned char color;
  unsigned int ind;
  Rect rect;
  float weit;
  for(unsigned long y = 0; y < h; ++y) {
    if ((y+1)*rh > height) {
      break;
    }
    for(unsigned long x = 0; x < w; ++x) {
      if ((x+1)*rw > width) {
	break;
      }
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
      printf("\x1B[38;5;%im%c\x1B[0m", color, ASCII[ind]);
    }
    printf("\n");
  }
  fflush(stdout);
}

int main(int argc, char** argv) {
  struct NSVGimage* image;
  unsigned long w, h;
  if (argc < 2) {
    printf("Usage: %s <svg> [<width> [<height>]]\n", argv[0]);
    return 1;
  }
  image = nsvgParseFromFile(argv[1], "px", 96);
  if (image == NULL) {
    printf("Error loading image %s\n", argv[1]);
    return 1;
  }
  if (argc < 3) {
    w = image->width;
  } else {
    w = atoi(argv[2]);
  }
  if (argc < 4) {
    h = image->height;
  } else {
    h = atoi(argv[3]);
  }
   
  unsigned char* img = new unsigned char[w*h*4];
  // Rasterize
  float scalex = ((float)w)/image->width;
  float scaley = ((float)h)/image->height;
  NSVGrasterizer* rast = nsvgCreateRasterizer();
  nsvgRasterizeStretched(rast, image, 0,0,scalex, scaley, img, w, h, w*4);
  nsvgDeleteRasterizer(rast);
  printArt(img, image->width * scalex, image->height * scaley, w, h);
  delete[] img;
  return 0;
}
