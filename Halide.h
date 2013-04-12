#ifndef __HALIDE_H__
#define __HALIDE_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

namespace Halide {
  struct Var {
    unsigned int lower;
    unsigned int upper;
  };

  template<typename T>
    class Image {
    public:
      unsigned int s0;
      T* base;

      // operators
      void operator=(Image &other) {
        base = other.base;
        s0 = other.s0;
      }

      T &operator()(int a, int b) {
        return base[a*s0 + b];
      }

      T operator()(int a, int b) const {
        return base[a*s0 + b];
      }
    };

  template <typename T>
    T inline max(T a, T b) {
      if (a >= b) return a;
      return b;
    }

  template <typename T>
    T inline min(T a, T b) {
      if (a <= b) return a;
      return b;
    }

  template <typename T>
    T inline clamp(T a, T b, T c) {
      if (a <= b) return b;
      if (a >= c) return c;
      return a;
    }

  template <typename T>
    T inline select(unsigned int c, T a, T b) {
      if (c) return a;
      return b;
    }
};

#endif
