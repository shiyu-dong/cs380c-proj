#include "Halide.h"
#include <assert.h>

using namespace Halide;

void Func_f(Var Var0, Var Var1, Image<int>& out) {
  // TODO: is this size calculation always correct?
  // do we consider input image size?
  out.s0 = Var0.upper - Var0.lower;
  out.s1 = Var1.upper - Var1.lower;
  // image data type informed by compiler
  out.base = new int[out.s0*out.s1];

  for(int x=Var1.lower; x<Var1.upper; x+=Var1.step) {
    for(int y=Var1.lower; y<Var1.upper; y+=Var1.step) {
      out.base[x*out.s0+y] = max(x, y);
    }
  }
  return;
}

int main(int argc, char **argv) {
    Var x, y;
    Func f, g, h;

    printf("Defining function...\n");

    // changed by compiler
    //f(x, y) = max(x, y);
    f = (void*)Func_f;

    printf("Realizing function...\n");

    // changed by compiler
    //Image<int> imf = f.realize(32, 32);
    x.lower = 0;
    x.upper = 32;
    x.step = 1;
    y.lower = 0;
    y.upper = 32;
    y.step = 1;

    Image<int> imf;

    (*(void(*)(Var, Var, Image<int>&))f)(x, y, imf);

    for(int i=0; i<32; i++) {
      for(int j=0; j<32; j++) {
        assert(imf.base[i*32+j] == max(i, j));
      }
    }

    return 0;
}
