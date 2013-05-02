#include <sys/time.h>
#include "halide/Halide.h"
using namespace Halide;

#define size 8192
int main(int argc, char **argv) {
  Var x, y, xi, yi;
  struct timeval start;
  struct timeval end;

  Func input, blur_x, blur;

  Image<double> in(size, size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      in(j,i) = (double)rand() / RAND_MAX;
    }
  }

  gettimeofday(&start, NULL);

  ////The algorithm
  input(x, y) = in(clamp(x, 0, size-1), clamp(y, 0, size-1));
  blur_x(x, y) = (input(x-1, y) + input(x, y) + input(x+1, y))/3;
  blur(x, y) = (blur_x(x, y-1) + blur_x(x, y) + blur_x(x, y+1))/3;

  // The schedule
  input.root();

  blur_x.tile(x, y, xi, yi, 128, 128).parallel(y);
  blur.tile(x, y, xi, yi, 128, 128).parallel(y);

  //output
  Image<double> output = blur.realize(size, size);

  gettimeofday(&end, NULL);

  printf("time: %ld\n", (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec));

  printf("Done!\n");

  return 0;
}
