#include "halide/Halide.h"
using namespace Halide;

#define size 256

int main() {

  Var x, y;
  Func f, gx, gy, G, theta;

  Image<float> input(size, size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      input(j,i) = (float)rand() / RAND_MAX;
    }
  }

  f(x, y) = input(clamp(x, 0, size-1), clamp(y, 0, size-1));

  gx(x, y) = f(x-1,y-1) + 2.0f*f(x-1,y) + f(x-1,y+1) - f(x+1,y+1) - 2.0f*f(x+1,y) - f(x+1,y+1);
  gy(x, y) = f(x-1,y-1) + 2.0f*f(x,y-1) + f(x+1,y-1) - f(x-1,y+1) - 2.0f*f(x,y+1) - f(x+1,y+1);
  G(x, y) = sqrt(gx(x,y)*gx(x,y) + gy(x,y)*gy(x,y));
//  theta(x, y) = atan(gy(x, y)/gx(x, y));

  Image<float> out = G.realize(size, size);
}