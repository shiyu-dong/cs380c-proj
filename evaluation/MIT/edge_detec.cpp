#include <sys/time.h>
#include "halide/Halide.h"
using namespace Halide;

#define size 8192

int main() {
  Var x, y, xi, yi;
  Func f, gx, gy, G, theta;
  struct timeval start;
  struct timeval end;

  Image<float> input(size, size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      input(j,i) = (float)rand() / RAND_MAX;
    }
  }

  gettimeofday(&start, NULL);

  f(x, y) = input(clamp(x, 0, size-1), clamp(y, 0, size-1));

  gx(x, y) = f(x-1,y-1) + 2.0f*f(x-1,y) + f(x-1,y+1) - f(x+1,y+1) - 2.0f*f(x+1,y) - f(x+1,y+1);
  gy(x, y) = f(x-1,y-1) + 2.0f*f(x,y-1) + f(x+1,y-1) - f(x-1,y+1) - 2.0f*f(x,y+1) - f(x+1,y+1);
  G(x, y) = sqrt(gx(x,y)*gx(x,y) + gy(x,y)*gy(x,y));
//  theta(x, y) = atan(gy(x, y)/gx(x, y));

  f.root();

  gx.tile(x, y, xi, yi, 128, 128).parallel(y);
  gy.tile(x, y, xi, yi, 128, 128).parallel(y);
  G.tile(x, y, xi, yi, 128, 128).parallel(y);

  Image<float> out = G.realize(size, size);

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
