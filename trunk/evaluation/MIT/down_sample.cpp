#include "halide/Halide.h"
using namespace Halide;
#define size 2048

int main() {

  Var x, y;
  Func downx, downy, f;

  Image<double> input(size, size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      input(j,i) = (double)rand() / RAND_MAX;
    }
  }

  // Downsample with a 1 3 5 3 1 filter
  f(x, y) = input(clamp(x, 0, size-1), clamp(y, 0, size-1));
  downx(x, y) = (f(x-2,y) + 3.0f*f(x-1,y) + 5.0f*f(x,y) + 3.0f*f(x+1, y) + f(x+2, y)) / 13.0f;    
  downy(x, y) = (downx(x, y-2) + 3.0f*downx(x, y-1) + 5.0f*downx(x, y) + 3.0f*downx(x,y+1) + downx(x,y+2)) / 13.0f;

  Image<double> out = downy.realize(size, size);
  printf("Success!\n");
}
