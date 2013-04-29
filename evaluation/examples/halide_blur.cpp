#include "halide/Halide.h"
using namespace Halide;

int main(int argc, char **argv) {

  Image<int> input(16, 2);
  Func blur_x("blur_x"), blur_y("blur_y");
  Var x("x"), y("y"), xi("xi"), yi("yi");

  // The algorithm
  blur_x(x, y) = (input(x-1, y) + input(x, y) + input(x+1, y))/3;
  blur_y(x, y) = (blur_x(x, y-1) + blur_x(x, y) + blur_x(x, y+1))/3;
  
  // How to schedule it
  /*
  blur_y.tile(x, y, xi, yi, 128, 32);
  blur_y.vectorize(xi, 8);
  blur_y.parallel(y);
  blur_x.chunk(x);
  blur_x.vectorize(x, 8);
  */

  //blur_y.split(y, y, yi, 8).parallel(y).vectorize(x, 8);
  //blur_x.chunk(y, yi).vectorize(x, 8);

  //blur_y.compileToFile("halide_blur"); 
  Image<double> output = blur_y.realize(16, 2);
  return 0;
}
