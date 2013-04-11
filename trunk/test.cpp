#include "Halide.h"
#include <assert.h>
#include <iostream>

#define RESULT_TYPE int
using namespace Halide;

int main(int argc, char **argv) {
    Var x, y;
    Func f;

    printf("Defining function...\n");

    //f.realize(32, 32) && f(x, y)
    x.lower = 0;
    x.upper = 32;
    y.lower = 0;
    y.upper = 32;

    //Image<int> imf = f; or user specify result type
    Image<RESULT_TYPE> f;
    //f(x, y)
    f.s0 = x.upper - x.lower;
    f.s1 = 1;
    f.base = new RESULT_TYPE[(x.upper-x.lower)*(y.upper-y.lower)];

    //f(x, y) = max(x, y);
    for(int x_ite=x.lower; x_ite<x.upper; x_ite++) {
      for(int y_ite=y.lower; y_ite<y.upper; y_ite++) {
        f(x_ite, y_ite) = max(x_ite, y_ite);
      }
    }

    //f(x, y) = max(x, y);
    //Image<int> imf = f
    Image<int> imf;
    imf = f;

    printf("Realizing function...\n");

    // changed by compiler

    for(int i=0; i<32; i++) {
      for(int j=0; j<32; j++) {
        assert(imf.base[i*32+j] == max(i, j));
      }
    }

    return 0;
}
