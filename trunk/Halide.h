#ifndef __HALIDE_H__
#define __HALIDE_H__

#include <stdint.h>
//#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
//#include <immintrin.h>

namespace Halide {
  struct Var {
    unsigned int lower;
    unsigned int upper;
  };

  template<typename T>
    class Image {
    public:
      unsigned int s0;
      unsigned int s1;
      T* base;

      // constructor
      Image() {
        s0 = 1;
        s1 = 1;
      }

      Image(unsigned int x) : s0(x), s1(1) {
        base = (T*)calloc(32, sizeof(T)*x);
        //base = (T*)memalign(32, sizeof(T)*x);
      }

      Image(unsigned int x, unsigned int y) : s0(x), s1(y) {
        base = (T*)calloc(32, sizeof(T)*x*y);
        //base = (T*)memalign(32, sizeof(T)*x*y);
      }

      Image(unsigned int x, unsigned int y, unsigned int z) : s0(x), s1(y) {
        base = (T*)calloc(32, sizeof(T)*x*y*z);
        //base = (T*)memalign(32, sizeof(T)*x*y*z);
      }

      // operators
      void operator=(Image &other) {
        base = other.base;
        s0 = other.s0;
        s1 = other.s1;
      }

      T operator()(unsigned int a) const {
        return base[a];
      }
      T &operator()(unsigned int a) {
        return base[a];
      }
      T* getP(unsigned int a) {
        return base+a;
      }

      T operator()(unsigned int a, unsigned int b) const {
        return base[b*s0 + a];
      }
      T &operator()(unsigned int a, unsigned int b) {
        return base[b*s0 + a];
      }
      T* getP(unsigned int a, unsigned int b) {
        return base+b*s0+a;
      }

      T operator()(unsigned int a, unsigned int b, unsigned int c) const {
        return base[c*s0*s1 + b*s1 + a];
      }
      T &operator()(unsigned int a, unsigned int b, unsigned int c) {
        return base[c*s0*s1 + b*s1 + a];
      }
      T* getP(unsigned int a, unsigned int b, unsigned int c) {
        return base + c*s0*s1 + b*s1 + a;
      }
    };

  class RDom {
    public:
      int x, y, z;
      RDom(int a, int b) {}
      RDom(int a, int b, int c, int d) {}
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
