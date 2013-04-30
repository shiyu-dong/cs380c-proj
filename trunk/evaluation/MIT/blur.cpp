#include "halide/Halide.h"
using namespace Halide;

#define size 256
int main(int argc, char **argv) {
  Var x, y, xi, yi;

  Func input, blur_x, blur;

  Image<double> in(size, size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      in(j,i) = (double)rand() / RAND_MAX;
    }
  }

  ////The algorithm
  input(x, y) = in(clamp(x, 0, size-1), clamp(y, 0, size-1));
  blur_x(x, y) = (input(x-1, y) + input(x, y) + input(x+1, y))/3;
  blur(x, y) = (blur_x(x, y-1) + blur_x(x, y) + blur_x(x, y+1))/3;

  // The schedule
  input.root();

  blur_x.tile(x, y, xi, yi, 128, 128).vectorize(xi, 8).parallel(y);
  blur.tile(x, y, xi, yi, 128, 128).vectorize(yi, 8).parallel(y);

  //output
  Image<double> output = blur.realize(size, size);

  //for (int i = 0; i < size; i++) {
  //  for (int j = 0; j < size; j++) {
  //    printf("%f ", output(i, j));
  //  }
  //  printf("\n");
  //}
  printf("Success!\n");

  return 0;
}
