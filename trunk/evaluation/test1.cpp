#include "halide/Halide.h"
using namespace Halide;

int main(int argc, char **argv) {
  Var x, y, xi, yi;
  const int size = 1024;

  //input
  Image<double> input(size, size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      input(j,i) = (double)rand() / RAND_MAX;
    }
  }

  Func blur_x, blur_y, blur_all;

  ////The algorithm
  blur_x(x, y) = (input(x-1, y) + input(x, y) + input(x+1, y))/3;
  blur_y(x, y) = (blur_x(x, y-1) + blur_x(x, y) + blur_x(x, y+1))/3;
  blur_all(x, y) = (blur_y(x-1, y) + blur_y(x, y-1) + blur_y(x+1, y) + blur_y(x, y+1) + blur_y(x-1, y-1) + blur_y(x-1, y+1) + blur_y(x+1, y+1) + blur_y(x+1, y-1))/8;

  // The schedule
  blur_x.tile(x, y, xi, yi, 128, 128);
  blur_x.vectorize(xi, 8);
  blur_x.parallel(x);

  blur_y.tile(x, y, xi, yi, 128, 128);
  blur_y.vectorize(yi, 8);
  blur_y.parallel(y);

  blur_all.tile(x, y, xi, yi, 128, 128);
  blur_all.vectorize(yi, 8);
  blur_all.parallel(y);

  //output
  Image<double> output = blur_all.realize(size, size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      printf("%f ", output(j, i));
    }
    printf("\n");
  }

  return 0;
}
