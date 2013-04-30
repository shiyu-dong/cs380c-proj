#include "halide/Halide.h"
using namespace Halide;
#define size 1024

//int main(int argc, char **argv) {
//
//  //output
//  Image<double> output = blur_y.realize(size, size);
//  for (int y = 1; y < size; y++) {
//    for (int x = 0; x < size; x++) {
//      printf("%f ", output(x, y));
//    }
//    printf("\n");
//  }
//
//  return 0;
//}
int main(int argc, char** argv){

  Var x, y, xi, yi;
  Func input_func, blur_x, blur_y, blur_all;

  Image<double> input(size, size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      input(j,i) = (double)rand() / RAND_MAX;
    }
  }
  input_func(x, y) = input(clamp(x, 0, size-1), clamp(y, 0, size-1));

  //The algorithm
  blur_x(x, y) = (input_func(x-1, y) + input_func(x, y) + input_func(x+1, y))/3;
  blur_y(x, y) = (blur_x(x, y-1) + blur_x(x, y) + blur_x(x, y+1))/3;
  blur_all(x, y) = (blur_y(x-1, y) + blur_y(x, y-1) + blur_y(x+1, y) + blur_y(x, y+1) + blur_y(x-1, y-1) + blur_y(x-1, y+1) + blur_y(x+1, y+1) + blur_y(x+1, y-1))/8;

  // The schedule
  blur_x.tile(x, y, xi, yi, 128, 128).vectorize(xi, 8).parallel(x);
  blur_y.tile(x, y, xi, yi, 128, 128).vectorize(yi, 8).parallel(y);

  Image<double> output = blur_all.realize(size,size);
  for (int y = 1; y < size; y++) {
    for (int x = 0; x < size; x++) {
      printf("%f ", output(x, y));
    }
    printf("\n");
  }

  return 0;
}
