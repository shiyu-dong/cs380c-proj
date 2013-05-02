#include <sys/time.h>
#include "halide/Halide.h"
using namespace Halide;
#define size 8192

int main() {

  Var x, y, xi, yi;
  Func downx, downy, f;
  struct timeval start;
  struct timeval end;

  Image<double> input(size, size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
  //    input(j,i) = (double)rand() / RAND_MAX;
      input(i,j) = i*size+j;
    }
  }

  gettimeofday(&start, NULL);
  // Downsample with a 1 3 5 3 1 filter
  f(x, y) = input(clamp(x, 0, size-1), clamp(y, 0, size-1));
  downx(x, y) = (f(x-2,y) + 3.0f*f(x-1,y) + 5.0f*f(x,y) + 3.0f*f(x+1, y) + f(x+2, y)) / 13.0f;    
  downy(x, y) = (downx(x, y-2) + 3.0f*downx(x, y-1) + 5.0f*downx(x, y) + 3.0f*downx(x,y+1) + downx(x,y+2)) / 13.0f;

  f.parallel(y);
  downx.tile(x, y, xi, yi, 128, 128).parallel(y);
  downy.tile(x, y, xi, yi, 128, 128).parallel(y);

  Image<double> out = downy.realize(size, size);

  gettimeofday(&end, NULL);
  printf("time: %ld\n", (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec));

//  for (int i = 0; i < size; i++) {
//    for (int j = 0; j < size; j++) {
//      printf("%lf ", out(i, j));
//    }
//    printf("\n");
//  }

  printf("Success!\n");
}
