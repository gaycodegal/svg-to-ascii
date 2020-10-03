#include <stdlib.h>
#include <stdio.h>
#include "ascii.hh"

int main(int argc, char** argv) {
  unsigned long w = 0, h = 0;
  if (argc < 2) {
    printf("Usage: %s <svg filename> [<width> [<height>]]\n", argv[0]);
    return 1;
  }

  if (argc > 2) {
    w = atoi(argv[2]);
  }

  if (argc > 3) {
    h = atoi(argv[3]);
  }
  
  Rect r = {0, 0, w, h/2};
  AsciiImage image(argv[1], r, argc < 3, argc < 4);
  image.print();
  
  return 0;
}
