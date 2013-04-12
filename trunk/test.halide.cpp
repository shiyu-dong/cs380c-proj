#include "Halide.h"
#include <assert.h>
#include <iostream>

#define RESULT_TYPE int
#define Func Image<RESULT_TYPE>
using namespace Halide;

int main(int argc, char **argv) {
    Func f, g;


    f.base = new RESULT_TYPE[32*32];
    f.s0 = 32;

    for(unsigned int x=0; x<32; x++) {
      for(unsigned int y=0; y<32; y++) {
        f(x, y) = max(x, y);
      }
    }

    g.base = new RESULT_TYPE[32*32];
    g.s0 = 32;

    for(unsigned int x=0; x<32; x++) {
      for(unsigned int y=0; y<32; y++) {
        g(x, y) = min(x, y);
      }
    }


    Image<int> imf = f;
    Image<int> img = g;

    for(int i=0; i<32; i++) {
      for(int j=0; j<32; j++) {
        assert(imf.base[i*32+j] == max(i, j));
      }
    }

    return 0;
}
