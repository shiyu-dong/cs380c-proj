#include "halide/Halide.h"
using namespace Halide;

#define size 1024
int main(int argc, char **argv) {
  Var x, y, xi, yi;

  //input
  Image<double> input(size, size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      input(i,j) = (double)rand() / RAND_MAX;
    }
  }

  Func blur_x, blur;

  ////The algorithm
  blur_x(x, y) = (input(x-1, y) + input(x, y) + input(x+1, y))/3;
  blur(x, y) = (blur_x(x, y-1) + blur_x(x, y) + blur_x(x, y+1))/3;

  // The schedule
  //blur_x.tile(x, y, xi, yi, 128, 128).vectorize(xi, 8).parallel(x);
  //blur_y.tile(x, y, xi, yi, 128, 128).vectorize(yi, 8).parallel(y);
  
//  blur.tile(x, y, xi, yi, 1024, 1024);

  //output
  Image<double> output = blur.realize(1000, 1000);

//  for (int i = 0; i < size; i++) {
//    for (int j = 0; j < size; j++) {
//      printf("%f ", output(i, j));
//    }
//    printf("\n");
//  }

  return 0;
}
