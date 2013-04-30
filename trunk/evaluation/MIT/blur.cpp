#include "halide/Halide.h"
using namespace Halide;

#define size 8
int main(int argc, char **argv) {
  Var x, y, xi, yi;

  Func input, blur_x, blur;

  ////The algorithm
  input(x, y) = cast<float>(x*y+0.2);
  blur_x(x, y) = (input(x-1, y) + input(x, y) + input(x+1, y))/3;
  blur(x, y) = (blur_x(x, y-1) + blur_x(x, y) + blur_x(x, y+1))/3;

  // The schedule
  //blur_x.tile(x, y, xi, yi, 128, 128).vectorize(xi, 8).parallel(x);
  //blur_y.tile(x, y, xi, yi, 128, 128).vectorize(yi, 8).parallel(y);
  
  blur.parallel(y);

  //output
  Image<float> output = blur.realize(size, size);

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      printf("%f ", output(i, j));
    }
    printf("\n");
  }

  return 0;
}
