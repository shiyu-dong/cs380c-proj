#ifndef MLVAL_H
#define MLVAL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <tr1/memory>
using std::tr1::shared_ptr;

class MLVal {
public:
    static MLVal find(const char *name);
    MLVal() {}
    MLVal(const char *);
    MLVal(const std::string &);
    MLVal(int);
    MLVal(uint32_t);
    MLVal(float);
    MLVal(double);
    MLVal(void *);
    MLVal(const MLVal &);
    MLVal operator()() const;
    MLVal operator()(const MLVal &a) const;
    MLVal operator()(const MLVal &a, const MLVal &b) const;
    MLVal operator()(const MLVal &a, const MLVal &b, const MLVal &c) const;
    MLVal operator()(const MLVal &a, const MLVal &b, const MLVal &c, const MLVal &d) const;
    MLVal operator()(const MLVal &a, const MLVal &b, const MLVal &c, const MLVal &d, const MLVal &e) const;
    MLVal operator()(const MLVal &a, const MLVal &b, const MLVal &c, const MLVal &d, const MLVal &e, const MLVal &f) const;

    operator std::string();

    operator bool() const {return contents.get();}

    void *asVoidPtr() const;

    static void unpackPair(const MLVal &input, MLVal &first, MLVal &second);
 private:
    struct Contents;
    shared_ptr<Contents> contents;
};

// Macro to help dig out functions
#define ML_FUNC0(n)                                            \
    MLVal n() {                                                \
        static MLVal callback;                                 \
        if (!callback) callback = MLVal::find(#n);             \
        return callback();                                     \
    }

#define ML_FUNC1(n)                                            \
    MLVal n(const MLVal &x) {                                         \
        static MLVal callback;                                 \
        if (!callback) callback = MLVal::find(#n);             \
        return callback(x);                                    \
    }

#define ML_FUNC2(n)                                                    \
    MLVal n(const MLVal &x, const MLVal &y) {                                        \
        static MLVal callback;                                         \
        if (!callback) callback = MLVal::find(#n);                     \
        return callback(x, y);                                         \
    }

#define ML_FUNC3(n)                                                    \
    MLVal n(const MLVal &x, const MLVal &y, const MLVal &z) {                               \
        static MLVal callback;                                         \
        if (!callback) callback = MLVal::find(#n);                     \
        return callback(x, y, z);                                      \
    }


#define ML_FUNC4(n)                                                    \
    MLVal n(const MLVal &x, const MLVal &y, const MLVal &z, const MLVal &w) {                      \
        static MLVal callback;                                         \
        if (!callback) callback = MLVal::find(#n);                     \
        return callback(x, y, z, w);                                   \
    }


#define ML_FUNC5(n)                                                    \
    MLVal n(const MLVal &a, const MLVal &b, const MLVal &c, const MLVal &d, const MLVal &e) {             \
        static MLVal callback;                                         \
        if (!callback) callback = MLVal::find(#n);                     \
        return callback(a, b, c, d, e);                                \
    }

#define ML_FUNC6(n)                                                    \
    MLVal n(const MLVal &a, const MLVal &b, const MLVal &c, const MLVal &d, const MLVal &e, const MLVal &f) {    \
        static MLVal callback;                                         \
        if (!callback) callback = MLVal::find(#n);                     \
        return callback(a, b, c, d, e, f);                             \
    }
 
#endif
#ifndef HALIDE_UTIL_H
#define HALIDE_UTIL_H

#include <string>

namespace Halide {
    // Generate a unique name
    std::string uniqueName(char prefix);
    
    // Make ML lists
    MLVal makeList();
    MLVal addToList(const MLVal &list, const MLVal &item);

    std::string int_to_str(int);          // Connelly: workaround function for ostringstream << int failing in Python binding
}

#endif
#ifndef HALIDE_VAR_H
#define HALIDE_VAR_H

#include <assert.h>

namespace Halide {

    class Expr;
    class UniformImage;
    class DynImage;
    class RDom;

    // A variable
    class Var {
    public:
        Var() : _name(uniqueName('v')) { }
        Var(const std::string &name) : _name(name) { }

        const std::string &name() const {return _name;}

        bool operator==(const Var &other) const {
            return name() == other.name();
        }
        
    private:
        std::string _name;
    };

    // A reduction variable
    class RVar {
    public:
        // Make a reduction variable 
        RVar();
        RVar(const RDom &, const Expr &min, const Expr &size);
        RVar(const RDom &, const Expr &min, const Expr &size, const std::string &name);
       
        //void bound(const Expr &min, const Expr &size);   // Connelly: bound() is not defined in Var.cpp

        const Expr &min() const;
        const Expr &size() const;
        const std::string &name() const;
        bool operator==(const RVar &other) const;

        operator Var() {return Var(name());}
	const RDom &domain() const;

	bool isDefined() const {return (bool)contents;}
    private:

        struct Contents;
        shared_ptr<Contents> contents;
    };

    // A reduction domain
    class RDom {
    public:
	RDom();

	RVar x, y, z, w;

	RDom(const UniformImage &im);
	RDom(const DynImage &im);

	RDom(const Expr &min, const Expr &size);
	RDom(const Expr &min1, const Expr &size1,
	     const Expr &min2, const Expr &size2);
	RDom(const Expr &min1, const Expr &size1,
	     const Expr &min2, const Expr &size2,
	     const Expr &min3, const Expr &size3);
	RDom(const Expr &min1, const Expr &size1,
	     const Expr &min2, const Expr &size2,
	     const Expr &min3, const Expr &size3,
	     const Expr &min4, const Expr &size4);
	RDom(const Expr &min, const Expr &size,
	     const std::string &name);
	RDom(const Expr &min1, const Expr &size1,
	     const Expr &min2, const Expr &size2,
	     const std::string &name);
	RDom(const Expr &min1, const Expr &size1,
	     const Expr &min2, const Expr &size2,
	     const Expr &min3, const Expr &size3,
	     const std::string &name);
	RDom(const Expr &min1, const Expr &size1,
	     const Expr &min2, const Expr &size2,
	     const Expr &min3, const Expr &size3,
	     const Expr &min4, const Expr &size4,
	     const std::string &name);

        bool operator==(const RDom &other) const;

	const RVar &operator[](int i) const;
	bool isDefined() const {return (bool)contents;}

	int dimensions() const;

	operator Var() {assert(dimensions() == 1); return Var(x);}

      private:
	struct Contents;
	shared_ptr<Contents> contents;
    };

}

#endif
#ifndef HALIDE_TYPE_H
#define HALIDE_TYPE_H

#include <sstream>

namespace Halide {
    // Possible types for image data
    class Type {
      public:
        MLVal mlval;
        unsigned char bits;
        enum {FLOAT = 0, INT = 1, UINT = 2} code;
        bool isInt() {return code == INT;}
        bool isUInt() {return code == UINT;}
        bool isFloat() {return code == FLOAT;}
        bool operator==(const Type &other) const {
            return bits == other.bits && code == other.code;
        }
        bool operator!=(const Type &other) const {
            return !(*this == other);
        }
        std::string str() const {
            std::string codes[] = {"f", "s", "u"};
            std::ostringstream ss;
            ss << ((code < 3 && code >= 0) ? codes[code] : "malformed_type_");
            ss << (int)(bits);
            return ss.str();
        }        
    };

    Type Float(unsigned char bits);
    Type Int(unsigned char bits);
    Type UInt(unsigned char bits);

    template<typename T> Type TypeOf();
    template<> Type TypeOf<float>();
    template<> Type TypeOf<double>();
    template<> Type TypeOf<unsigned char>();
    template<> Type TypeOf<unsigned short>();
    template<> Type TypeOf<unsigned int>();
    template<> Type TypeOf<bool>();
    template<> Type TypeOf<char>();
    template<> Type TypeOf<short>();
    template<> Type TypeOf<int>();
    template<> Type TypeOf<signed char>();
}


#endif
#ifndef HALIDE_UNIFORM_H
#define HALIDE_UNIFORM_H

#include <assert.h>

namespace Halide {

    // Dynamically and statically typed uniforms

    class DynUniform {
    public:
        DynUniform(Type t) : contents(new Contents(t, uniqueName('u'))) {}
        DynUniform(Type t, const std::string &name) : contents(new Contents(t, name)) {}

        Type type() const {return contents->type;}
        const std::string &name() const {return contents->name;}

        template<typename T>
        void set(T v) {
            assert(TypeOf<T>() == type());
            contents->val = 0; 
            T *ptr = (T *)(&(contents->val));
            *ptr = v;
        }

        bool operator==(const DynUniform &other) {
            return contents == other.contents;
        }

        void *data() const {
            return (&contents->val);
        }

    private:
        struct Contents {
            Contents(Type t, const std::string &n) : val(0), name(n), type(t) {}
            int64_t val;
            const std::string name;
            Type type;
        };
        shared_ptr<Contents> contents;
    };

    template<typename T>
    class Uniform {
    public:
        Uniform(const T &v = 0) : u(TypeOf<T>()) {
            u.set(v);
        }

        Uniform(const std::string &name, const T &v = 0) : u(TypeOf<T>(), name) {
            u.set(v);
        }

        void operator=(T v) {
            u.set(v);
        }
        
        operator DynUniform() const {
            return u;
        }

        Type type() const {return TypeOf<T>();}
        const std::string &name() const {return u.name();}
        void *data() const {return u.data();}

        //operator size_t() const { return (size_t)data(); }

    private:
        DynUniform u;
    };

}

#endif
#ifndef HALIDE_EXPR_H
#define HALIDE_EXPR_H

#include <string>
#include <vector>


namespace Halide {

    class Var;
    class RVar;
    class RDom;
    class FuncRef;
    class DynUniform;
    class ImageRef;
    class Type;
    class DynImage;
    class Func;
    class UniformImage;
    class UniformImageRef;
    
    template<typename T>
    class Uniform;
    
    // A node in an expression tree.
    class Expr {
    public:
        Expr();
        Expr(MLVal, Type);

        Expr(int32_t);
        Expr(unsigned);
        Expr(float);
        Expr(double);
        Expr(const Var &);
        Expr(const RVar &);
        Expr(const RDom &);
        Expr(const FuncRef &);
        Expr(const DynUniform &);
        Expr(const ImageRef &);
        Expr(const UniformImageRef &);
        Expr(const Func &);
        Expr(const Expr &);
        
        template<typename T> Expr(const Uniform<T> &u) : contents(Expr((DynUniform)u).contents) {}

        void operator+=(Expr);
        void operator-=(Expr);
        void operator*=(Expr);
        void operator/=(Expr);

        // declare that this node depends on something
        void child(Expr);

        // These calls are only used to inject dependence that isn't
        // implied by the way the expression was constructed
        void child(const UniformImage &);
        void child(const DynUniform &);
        void child(const DynImage &);
        void child(const Var &);
        void child(const Func &);

        const MLVal &node() const;
        const Type &type() const;
        const std::vector<DynUniform> &uniforms() const;
        const std::vector<DynImage> &images() const;
        const std::vector<Var> &vars() const;
	void setRDom(const RDom &dom);
	const RDom &rdom() const;
        const std::vector<Func> &funcs() const;
        const std::vector<UniformImage> &uniformImages() const;
        int implicitArgs() const;
        void addImplicitArgs(int);

        bool isVar() const;
        bool isRVar() const;
        bool isDefined() const;
        bool isImmediate() const;

        // For a scalar this is an empty vector
        // For a tuple this gives the shape of the tuple
        std::vector<int> &shape() const;

        // When an expression is captured and placed inside an
        // anonymous function body, any reduction vars become regular
        // vars to the anonymous function
        void convertRVarsToVars();
        
      private:
        struct Contents;
        shared_ptr<Contents> contents;
    };

    // Make a binary op node
    Expr operator+(Expr, Expr);
    Expr operator-(Expr);
    Expr operator-(Expr, Expr);
    Expr operator*(Expr, Expr);
    Expr operator/(Expr, Expr);
    Expr operator%(Expr, Expr);

    // Make a comparison node
    Expr operator>(Expr, Expr);
    Expr operator>=(Expr, Expr);
    Expr operator<(Expr, Expr);
    Expr operator<=(Expr, Expr);
    Expr operator!=(Expr, Expr);
    Expr operator==(Expr, Expr);

    // Logical operators
    Expr operator&&(Expr, Expr);
    Expr operator||(Expr, Expr);
    Expr operator!(Expr);

    // Calls to builtin functions
    Expr builtin(Type, const std::string &name);
    Expr builtin(Type, const std::string &name, Expr);
    Expr builtin(Type, const std::string &name, Expr, Expr);
    Expr builtin(Type, const std::string &name, Expr, Expr, Expr);
    Expr builtin(Type, const std::string &name, Expr, Expr, Expr, Expr);

    // Transcendentals and other builtins
    Expr sqrt(Expr);
    Expr sin(Expr);
    Expr cos(Expr);
    Expr pow(Expr, Expr);
    Expr exp(Expr);
    Expr log(Expr);
    Expr floor(Expr);
    Expr ceil(Expr);
    Expr round(Expr);
    Expr abs(Expr);

    // Make a debug node
    Expr debug(Expr, const std::string &prefix, const std::vector<Expr> &args);
    Expr debug(Expr, const std::string &prefix);
    Expr debug(Expr, const std::string &prefix, Expr a);
    Expr debug(Expr, const std::string &prefix, Expr a, Expr b);
    Expr debug(Expr, const std::string &prefix, Expr a, Expr b, Expr c);
    Expr debug(Expr, const std::string &prefix, Expr a, Expr b, Expr c, Expr d);
    Expr debug(Expr, const std::string &prefix, Expr a, Expr b, Expr c, Expr d, Expr e);

    // Make a ternary operator
    Expr select(Expr, Expr, Expr);

    Expr max(Expr, Expr);
    Expr min(Expr, Expr);
    Expr clamp(Expr, Expr, Expr);

    // Make a cast node
    Expr cast(Type, Expr);

    template<typename T>
    Expr cast(Expr e) {
        return cast(TypeOf<T>(), e);
    }


    // Convenience macros that lift functions that take regular types
    // into functions that take and return exprs, and call the original
    // function at runtime under the hood. See test/cpp/c_function for
    // example usage.
#define HalideExtern_0(rt, name)			\
    Halide::Expr name() {				\
    return Halide::builtin(TypeOf<rt>(), #name);	\
  }

#define HalideExtern_1(rt, name, t1) 					\
    Halide::Expr name(Halide::Expr a1) {				\
    assert(a1.type() == Halide::TypeOf<t1>() && "Type mismatch for argument 1 of " #name); \
    return Halide::builtin(Halide::TypeOf<rt>(), #name, a1);		\
  }

#define HalideExtern_2(rt, name, t1, t2) 				\
    Halide::Expr name(Halide::Expr a1, Halide::Expr a2) {		\
    assert(a1.type() == Halide::TypeOf<t1>() && "Type mismatch for argument 1 of " #name); \
    assert(a2.type() == Halide::TypeOf<t2>() && "Type mismatch for argument 2 of " #name); \
    return builtin(Halide::TypeOf<rt>(), #name, a1, a2);		\
  }

#define HalideExtern_3(rt, name, t1, t2, t3) 				\
    Halide::Expr name(Halide::Expr a1, Halide::Expr a2, Halide::Expr a3) { \
    assert(a1.type() == Halide::TypeOf<t1>() && "Type mismatch for argument 1 of " #name); \
    assert(a2.type() == Halide::TypeOf<t2>() && "Type mismatch for argument 2 of " #name); \
    assert(a3.type() == Halide::TypeOf<t3>() && "Type mismatch for argument 3 of " #name); \
    return builtin(Halide::TypeOf<rt>(), #name, a1, a2, a3);		\
  }

#define HalideExtern_4(rt, name, t1, t2, t3, t4) 			\
    Halide::Expr name(Halide::Expr a1, Halide::Expr a2, Halide::Expr a3, Halide::Expr a4) { \
    assert(a1.type() == Halide::TypeOf<t1>() && "Type mismatch for argument 1 of " #name); \
    assert(a2.type() == Halide::TypeOf<t2>() && "Type mismatch for argument 2 of " #name); \
    assert(a3.type() == Halide::TypeOf<t3>() && "Type mismatch for argument 3 of " #name); \
    assert(a4.type() == Halide::TypeOf<t4>() && "Type mismatch for argument 4 of " #name); \
    return builtin(Halide::TypeOf<rt>(), #name, a1, a2, a3, a4);	\
  }

}

#endif
#ifndef HALIDE_IMAGE_H
#define HALIDE_IMAGE_H

#include <stdint.h>

struct buffer_t;

namespace Halide {
    
    class Type;
  
    // The base image type with no typed accessors
    class DynImage {
    public:
        DynImage(const Type &t, int a);
        DynImage(const Type &t, int a, int b);
        DynImage(const Type &t, int a, int b, int c);
        DynImage(const Type &t, int a, int b, int c, int d);
        DynImage(const Type &t, std::vector<int> sizes);
        DynImage(const DynImage &other);

        Expr operator()(const Expr &a) const;
        Expr operator()(const Expr &a, const Expr &b) const;
        Expr operator()(const Expr &a, const Expr &b, const Expr &c) const;
        Expr operator()(const Expr &a, const Expr &b, const Expr &c, const Expr &d) const;

        const Type &type() const;
        int stride(int i) const;
        int size(int i) const;
        int dimensions() const;
        unsigned char *data() const;
        const std::string &name() const;
        struct buffer_t* buffer() const;
        void setRuntimeHooks(void (*copyToHostFn)(buffer_t *), void (*freeFn)(buffer_t *)) const;
        void copyToHost() const;
        void copyToDev() const;
        void markHostDirty() const;
        void markDevDirty() const;
        bool hostDirty() const;
        bool devDirty() const;
        
        // Convenience functions for typical interpretations of dimensions
        int width() const {return size(0);}
        int height() const {return size(1);}
        int channels() const {
            if (dimensions() > 2)
                return size(2);
            else
                return 1;
        }

        // Compare for identity (not equality of contents)
        bool operator==(const DynImage &other) const {
            return other.contents == contents;
        }

    private:
        struct Contents;
        shared_ptr<Contents> contents;
    };


    // The (typed) image type
    template<typename T>
    class Image {
    private:
        DynImage im;
        T *base;
        int s0, s1, s2, s3;

        void init() {
            im.markHostDirty();
            base = (T*)im.data();
            s0 = im.stride(0);
            if (im.dimensions() > 1) s1 = im.stride(1);
            if (im.dimensions() > 2) s2 = im.stride(2);
            if (im.dimensions() > 3) s3 = im.stride(3);            
        }

    public:
        Image(int a) : im(TypeOf<T>(), a) {init();}
        Image(int a, int b) : im(TypeOf<T>(), a, b) {init();}
        Image(int a, int b, int c) : im(TypeOf<T>(), a, b, c) {init();}
        Image(int a, int b, int c, int d) : im(TypeOf<T>(), a, b, c, d) {init();}
        Image(DynImage im) : im(im) {
            assert(TypeOf<T>() == im.type());
            im.copyToHost();
            init();
        }

        #if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
        Image(std::initializer_list<T> l) : im(TypeOf<T>(), l.size()) {
            init();
            int x = 0;
            for (auto &iter: l) {
                (*this)(x++) = iter;
            }
        }

        Image(std::initializer_list<std::initializer_list<T> > l) : im(TypeOf<T>(), l.begin()->size(), l.size()) {
            init();
            int y = 0;
            for (auto &row: l) {
                int x = 0;
                for (auto &elem: row) {
                    (*this)(x++, y) = elem;
                }
                y++;
            }
        }
	#endif

        operator DynImage() const {
            return im;
        }

        // Construct a load expression
        Expr operator()(const Expr &a) const {
            return im(a);
        }

        Expr operator()(const Expr &a, const Expr &b) const {
            return im(a, b);
        }

        Expr operator()(const Expr &a, const Expr &b, const Expr &c) const {
            return im(a, b, c);
        }

        Expr operator()(const Expr &a, const Expr &b, const Expr &c, const Expr &d) const {
            return im(a, b, c, d);
        }

        // Actually look something up in the image. Won't return anything
        // interesting if the image hasn't been evaluated yet.
        T &operator()(int a) {
            return base[a*s0];
        }
        
        T &operator()(int a, int b) {
            return base[a*s0 + b*s1];
        }
        
        T &operator()(int a, int b, int c) {
            return base[a*s0 + b*s1 + c*s2];
        }
        
        T &operator()(int a, int b, int c, int d) {
            return base[a*s0 + b*s1 + c*s2 + d*s3];
        }

        T operator()(int a) const {
            return base[a*s0];
        }
        
        T operator()(int a, int b) const {
            return base[a*s0 + b*s1];
        }
        
        T operator()(int a, int b, int c) const {
            return base[a*s0 + b*s1 + c*s2];
        }
        
        T operator()(int a, int b, int c, int d) const {
            return base[a*s0 + b*s1 + c*s2 + d*s3];
        }

        // Convenience functions for typical interpretations of dimensions
        int width() const {return im.width();}
        int height() const {return im.height();}
        int channels() const {return im.channels();}
        int size(int i) const {return im.size(i);}
        int dimensions() const {return im.dimensions();}
        unsigned char *data() const {return im.data();}
    };

    
    class ImageRef {
    public:
        ImageRef(const DynImage &im, const std::vector<Expr> &idx) : image(im), idx(idx) {}
        const DynImage image;
        const std::vector<Expr> idx;
    };       
    
    class UniformImage {
    public:
        UniformImage(const Type &t, int dims);
        UniformImage(const Type &t, int dims, const std::string &name);
        UniformImage(const UniformImage &);

        void operator=(const DynImage &image);
        unsigned char *data() const;

        Expr operator()(const Expr &a) const;
        Expr operator()(const Expr &a, const Expr &b) const;
        Expr operator()(const Expr &a, const Expr &b, const Expr &c) const;
        Expr operator()(const Expr &a, const Expr &b, const Expr &c, const Expr &d) const;
        Type type() const;
        const std::string &name() const;
        int dimensions() const;

        const DynImage &boundImage() const;

        const Expr &size(int i) const;
        const Expr &width() const {return size(0);}
        const Expr &height() const {return size(1);}
        const Expr &channels() const {return size(2);}

        // Compare for identity (not equality of contents)
        bool operator==(const UniformImage &other) const;
        
    private:
        struct Contents;
        shared_ptr<Contents> contents;
    };

    class UniformImageRef {
    public:
        UniformImageRef(const UniformImage &im, const std::vector<Expr> &idx) : image(im), idx(idx) {}
        const UniformImage image;
        const std::vector<Expr> idx;
    };

}

#endif
#ifndef HALIDE_FUNC_H
#define HALIDE_FUNC_H

#include <memory>
#include <string>


namespace Halide {
    
    bool use_gpu();

    class Func;
    class Var;

    // A function call (if you cast it to an expr), or a function definition lhs (if you assign an expr to it).
    class FuncRef {
    public:

        FuncRef(const Func &f);
        FuncRef(const Func &f, const Expr &a); 
        FuncRef(const Func &f, const Expr &a, const Expr &b);
        FuncRef(const Func &f, const Expr &a, const Expr &b, const Expr &c);
        FuncRef(const Func &f, const Expr &a, const Expr &b, const Expr &c, const Expr &d);
        FuncRef(const Func &f, const Expr &a, const Expr &b, const Expr &c, const Expr &d, const Expr &e);
        FuncRef(const Func &f, const std::vector<Expr> &args);
        FuncRef(const FuncRef &);

        // This assignment corresponds to definition. This FuncRef is
        // defined to have the given expression as its value.
        void operator=(const Expr &e);
        
        // Syntactic sugar for some reductions        
        void operator+=(const Expr &e);
        void operator*=(const Expr &e);
        void operator++(int) {*this += 1;}
        void operator--() {*this += -1;}

        // Make sure we don't directly assign an FuncRef to an FuncRef (but instead treat it as a definition)
        void operator=(const FuncRef &other) {*this = Expr(other);}

        const Func &f() const;
        const std::vector<Expr> &args() const;
        
    private:
        struct Contents;
        
        shared_ptr<Contents> contents;
    };

    class Func {
    public:
        Func();
        Func(const char *name);
        Func(const char *name, Type t);
        Func(const std::string &name);
        Func(const Type &t);
        Func(const std::string &name, Type t);

        // Define a function
        void define(const std::vector<Expr> &args, const Expr &rhs);
        void operator=(const Expr &rhs) {define(std::vector<Expr>(), rhs);}
        
        // Generate a call to the function (or the lhs of a definition)
        FuncRef operator()(const Expr &a) {return FuncRef(*this, a);}
        FuncRef operator()(const Expr &a, const Expr &b) {return FuncRef(*this, a, b);}
        FuncRef operator()(const Expr &a, const Expr &b, const Expr &c) {return FuncRef(*this, a, b, c);}     
        FuncRef operator()(const Expr &a, const Expr &b, const Expr &c, const Expr &d) {return FuncRef(*this, a, b, c, d);}  
        FuncRef operator()(const Expr &a, const Expr &b, const Expr &c, const Expr &d, const Expr &e) {return FuncRef(*this, a, b, c, d, e);}  
        FuncRef operator()(const std::vector<Expr> &args) {return FuncRef(*this, args);}

        // Generate an image from this function by Jitting the IR and running it.
        DynImage realize(int a);
        DynImage realize(int a, int b);
        DynImage realize(int a, int b, int c);
        DynImage realize(int a, int b, int c, int d);
        DynImage realize(std::vector<int> sizes);
        void realize(const DynImage &);

        /* If this function is a reduction, get a handle to its update
           step for scheduling */
        Func &update();

        /* These methods generate a partially applied function that
         * takes a schedule and modifies it. These functions get pushed
         * onto the scheduleTransforms vector, which is traversed in
         * order starting from an initial default schedule to create a
         * mutated schedule */
        Func &tile(const Var &, const Var &,
                   const Var &, const Var &,
                   const Expr &f1, const Expr &f2);
        Func &tile(const Var &, const Var &,
                   const Var &, const Var &, 
                   const Var &, const Var &, 
                   const Expr &f1, const Expr &f2);
        Func &rename(const Var &, const Var &);
        Func &reset();
        Func &vectorize(const Var &);
        Func &unroll(const Var &);
        Func &transpose(const Var &, const Var &);
        Func &chunk(const Var &);
        Func &root();
        Func &parallel(const Var &);
        Func &random(int seed);
        Func &vectorize(const Var &, int factor);
        Func &unroll(const Var &, int factor);
        Func &split(const Var &, const Var &, const Var &, const Expr &factor);
        Func &cuda(const Var &, const Var &);
        Func &cuda(const Var &, const Var &, const Var &, const Var &);
        Func &cudaTile(const Var &, int xFactor);
        Func &cudaTile(const Var &, const Var &, int xFactor, int yFactor);
        //Func &cuda(const Var &, const Var &, const Var &, const Var &, const Var &, const Var &);

        int autotune(int argc, char **argv, std::vector<int> sizes);

        bool operator==(const Func &other) const;

        /* The space of all living functions (TODO: remove a function
           from the environment when it goes out of scope) */
        static MLVal *environment;
        
        // Various properties of the function
        const Expr &rhs() const;
        const Type &returnType() const;
        const std::vector<Expr> &args() const;
        const std::string &name() const;
        const std::vector<MLVal> &scheduleTransforms() const;
        
        // Get the variable defining argument i
        const Var &arg(int i) const;

        std::string serialize();

        void compileJIT();
        void compileToFile(const std::string &name);

        void setErrorHandler(void (*)(char *));

        struct Arg {
            template<typename T>
            Arg(const Uniform<T> &u) : arg(Arg(DynUniform(u)).arg) {}
            template<typename T>
            Arg(const Image<T> &u) : arg(Arg(DynImage(u)).arg) {}
            Arg(const UniformImage &);
            Arg(const DynUniform &);
            Arg(const DynImage &);
            MLVal arg;
        };

        void compileToFile(const std::string &name, std::vector<Arg> args);

    private:
        struct Contents;

        MLVal lower();
        MLVal inferArguments();

        shared_ptr<Contents> contents;
    };

}

#endif
#ifndef HALIDE_REDUCTION_H
#define HALIDE_REDUCTION_H

#include <assert.h>
#include <vector>
#include <memory>

namespace Halide {
  
    // Reductions are anonymous functions that can be cast to an Expr to use
    class sum {
    public:
        sum(const Expr &body) {
            Func anon;
            std::vector<Expr> args(body.vars().size());
            for (size_t i = 0; i < body.vars().size(); i++) {
                args[i] = body.vars()[i];
            }
            Expr init = cast(body.type(), 0);
            init.addImplicitArgs(body.implicitArgs());
            anon(args) = init;
            anon(args) = anon(args) + body;
            call = anon(args);
        }

        operator Expr() {
            return call;
        }
    private:
        Expr call;
    };

    class product {
    public:
        product(const Expr &body) {
            Func anon;
            std::vector<Expr> args(body.vars().size());
            for (size_t i = 0; i < body.vars().size(); i++) {
                args[i] = body.vars()[i];
            }
            Expr init = cast(body.type(), 1);
            init.addImplicitArgs(body.implicitArgs());
            anon(args) = init;
            anon(args) = anon(args) * body;
            call = anon(args);
        }
        
        operator Expr() {
            return call;
        }
    private:
        Expr call;
    };

    class minimum {
      public:
        minimum(const Expr &body) {
            Func anon;
            std::vector<Expr> args(body.vars().size());
            for (size_t i = 0; i < body.vars().size(); i++) {
                args[i] = body.vars()[i];
            }
            Expr init = builtin(body.type(), "maxval_" + body.type().str());
            init.addImplicitArgs(body.implicitArgs());
            anon(args) = init;
            anon(args) = min(anon(args), body);
            call = anon(args);
        }
        
        operator Expr() {
            return call;
        }
      private:
        Expr call;
    };
    
    class maximum {
      public:
        maximum(const Expr &body) {
            Func anon;
            std::vector<Expr> args(body.vars().size());
            for (size_t i = 0; i < body.vars().size(); i++) {
                args[i] = body.vars()[i];
            }
            Expr init = builtin(body.type(), "minval_" + body.type().str());
            init.addImplicitArgs(body.implicitArgs());
            anon(args) = init;
            anon(args) = max(anon(args), body);
            call = anon(args);
        }
        
        operator Expr() {
            return call;
        }
      private:
        Expr call;
    };

}

#endif

#ifndef HALIDE_UNIFORM_H
#define HALIDE_UNIFORM_H

#include <assert.h>

namespace Halide {

    // Dynamically and statically typed uniforms

    class DynUniform {
    public:
        DynUniform(Type t) : contents(new Contents(t, uniqueName('u'))) {}
        DynUniform(Type t, const std::string &name) : contents(new Contents(t, name)) {}

        Type type() const {return contents->type;}
        const std::string &name() const {return contents->name;}

        template<typename T>
        void set(T v) {
            assert(TypeOf<T>() == type());
            contents->val = 0; 
            T *ptr = (T *)(&(contents->val));
            *ptr = v;
        }

        bool operator==(const DynUniform &other) {
            return contents == other.contents;
        }

        void *data() const {
            return (&contents->val);
        }

    private:
        struct Contents {
            Contents(Type t, const std::string &n) : val(0), name(n), type(t) {}
            int64_t val;
            const std::string name;
            Type type;
        };
        shared_ptr<Contents> contents;
    };

    template<typename T>
    class Uniform {
    public:
        Uniform(const T &v = 0) : u(TypeOf<T>()) {
            u.set(v);
        }

        Uniform(const std::string &name, const T &v = 0) : u(TypeOf<T>(), name) {
            u.set(v);
        }

        void operator=(T v) {
            u.set(v);
        }
        
        operator DynUniform() const {
            return u;
        }

        Type type() const {return TypeOf<T>();}
        const std::string &name() const {return u.name();}
        void *data() const {return u.data();}

        //operator size_t() const { return (size_t)data(); }

    private:
        DynUniform u;
    };

}

#endif
#ifndef HALIDE_TUPLE_H
#define HALIDE_TUPLE_H

#include <assert.h>
#include <vector>

namespace Halide {

    class Tuple {
    public:
	Tuple(const Expr &a) {
	    contents.push_back(a);
	}

        Tuple(const Expr &a, const Expr &b) {
	    contents.push_back(a);
	    contents.push_back(b);
	}

        Tuple(const Expr &a, const Expr &b, const Expr &c) {
	    contents.push_back(a);
	    contents.push_back(b);
	    contents.push_back(c);
	}

        Tuple(const Expr &a, const Expr &b, const Expr &c, const Expr &d) {
	    contents.push_back(a);
	    contents.push_back(b);
	    contents.push_back(c);
	    contents.push_back(d);
	}

        Tuple operator,(const Expr &e) const {
            return Tuple(contents, e);
        }

        Tuple operator,(const Tuple &other) const {
            assert(contents.size() == other.contents.size());
            return Tuple(Expr(*this), Expr(other));
        }

        operator Expr() const; 
        
    private:
        Tuple(const std::vector<Expr> &init, const Expr &last) : contents(init) {
            contents.push_back(last);
        }

        std::vector<Expr> contents;
    };

    Tuple operator,(const Expr & a, const Expr &b);
   
}

#endif
