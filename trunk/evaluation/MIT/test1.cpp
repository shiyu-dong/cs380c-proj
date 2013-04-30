#include "halide/Halide.h"
using namespace Halide;

int main(int argc, char **argv) {
  Var x, y;

  //input
  Func input;
  Func avg;

  ////The algorithm
  input(x, y) = cast<float>(y)*x;
  avg(x, y) = input(x-1, y)+input(x+1,y);

  //output
  Image<float> output = avg.realize(8, 8);

  return 0;
}
