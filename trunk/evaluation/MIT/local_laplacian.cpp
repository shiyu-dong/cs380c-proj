#include "halide/Halide.h"
using namespace Halide;

// Downsample with a 1 3 3 1 filter
Func downsample(Func f) {
    Var x, y;
    Func downx, downy;
    
    downx(x, y) = (f(2*x-1, y) + 3.0f * (f(2*x, y) + f(2*x+1, y)) + f(2*x+2, y)) / 8.0f;    
    downy(x, y) = (downx(x, 2*y-1) + 3.0f * (downx(x, 2*y) + downx(x, 2*y+1)) + downx(x, 2*y+2)) / 8.0f;

    return downy;
}

// Upsample using bilinear interpolation
Func upsample(Func f) {
    Var x, y;
    Func upx, upy;
    
    upx(x, y) = 0.25f * f((x/2) - 1 + 2*(x % 2), y) + 0.75f * f(x/2, y);
    upy(x, y) = 0.25f * upx(x, (y/2) - 1 + 2*(y % 2)) + 0.75f * upx(x, y/2);

    return upy;
    
}

int main(int argc, char **argv) {

    /* THE ALGORITHM */

    // Number of pyramid levels 
    int J = 8;

    // number of intensity levels
    Uniform<int> levels;
    // Parameters controlling the filter
    Uniform<float> alpha, beta;
    // Takes a 16-bit input
    UniformImage input(UInt(16), 3);

    // loop variables
    Var x, y, c, k;

    // Make the remapping function as a lookup table.
    Func remap;
    Expr fx = cast<float>(x) / 256.0f;
    remap(x) = alpha*fx*exp(-fx*fx/2.0f);
    
    // Convert to floating point
    Func floating;
    floating(x, y, c) = cast<float>(input(x, y, c)) / 65535.0f;
    
    // Set a boundary condition
    Func clamped;
    clamped(x, y, c) = floating(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1), c);
    
    // Get the luminance channel
    Func gray;
    gray(x, y) = 0.299f * clamped(x, y, 0) + 0.587f * clamped(x, y, 1) + 0.114f * clamped(x, y, 2);

    // Make the processed Gaussian pyramid. 
    Func gPyramid[J];
    // Do a lookup into a lut with 256 entires per intensity level
    Expr idx = gray(x, y)*cast<float>(levels-1)*256.0f;
    idx = clamp(cast<int>(idx), 0, (levels-1)*256);
    gPyramid[0](x, y, k) = beta*gray(x, y) + remap(idx - 256*k);
    for (int j = 1; j < J; j++) {
        gPyramid[j](x, y, k) = downsample(gPyramid[j-1])(x, y, k);
    }    

    // Get its laplacian pyramid
    Func lPyramid[J];
    lPyramid[J-1] = gPyramid[J-1];
    for (int j = J-2; j >= 0; j--) {
        lPyramid[j](x, y, k) = gPyramid[j](x, y, k) - upsample(gPyramid[j+1])(x, y, k);    
    }

    // Make the Gaussian pyramid of the input
    Func inGPyramid[J];
    inGPyramid[0] = gray;
    for (int j = 1; j < J; j++) {
        inGPyramid[j](x, y) = downsample(inGPyramid[j-1])(x, y);
    }        

    // Make the laplacian pyramid of the output
    Func outLPyramid[J];
    for (int j = 0; j < J; j++) {
        // Split input pyramid value into integer and floating parts
        Expr level = inGPyramid[j](x, y) * cast<float>(levels-1);
        Expr li = clamp(cast<int>(level), 0, levels-2);
        Expr lf = level - cast<float>(li);
        // Linearly interpolate between the nearest processed pyramid levels
        outLPyramid[j](x, y) = (1.0f - lf) * lPyramid[j](x, y, li) + lf * lPyramid[j](x, y, li+1);
    }
    
    // Make the Gaussian pyramid of the output
    Func outGPyramid[J];
    outGPyramid[J-1] = outLPyramid[J-1];
    for (int j = J-2; j >= 0; j--) {
        outGPyramid[j](x, y) = upsample(outGPyramid[j+1])(x, y) + outLPyramid[j](x, y);
    }    

    // Reintroduce color (Connelly: use eps to avoid scaling up noise w/ apollo3.png input)
    Func color;
    float eps = 0.01f;
    color(x, y, c) = outGPyramid[0](x, y) * (clamped(x, y, c)+eps) / (gray(x, y)+eps);
        
    Func output;
    // Convert back to 16-bit
    output(x, y, c) = cast<uint16_t>(clamp(color(x, y, c), 0.0f, 1.0f) * 65535.0f);



    /* THE SCHEDULE */


    // While normally we'd leave in just the best schedule per
    // architecture, here we kept track of everything we tried inside
    // a giant switch statement, to demonstrate how we go about
    // optimizing the schedule. The reference implementation on the
    // quad-core machine took 627 ms.

    // In any case, the remapping function should be a lut evaluated
    // ahead of time. It's so small relative to everything else that
    // its schedule really doesn't matter (provided we don't inline
    // it).
    remap.root();

    Var yi;

    // Times are for a quad-core core2, a 32-core nehalem, and a 2-core omap4 cortex-a9
    int schedule_num = 0;
    switch (schedule_num) {
    case 0:
        // As a baseline, breadth-first scalar: 1572, 1791, 9690 
        output.root();
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root();
            gPyramid[j].root();
            outGPyramid[j].root();
            if (j == J-1) break;
            lPyramid[j].root();
            outLPyramid[j].root();
        }
        break;        
    case 1:
        // parallelize each stage across outermost dimension: 769, 321, 5622 
        output.split(y, y, yi, 32).parallel(y);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().split(y, y, yi, 4).parallel(y);
            gPyramid[j].root().parallel(k);
            outGPyramid[j].root().split(y, y, yi, 4).parallel(y);
            if (j == J-1) break;
            lPyramid[j].root().parallel(k);
            outLPyramid[j].root().split(y, y, yi, 4).parallel(y);
        }
        break;        
    case 2:
        // Same as above, but also vectorize across x: 855, 288, 7004
        output.split(y, y, yi, 32).parallel(y).vectorize(x, 4);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
            gPyramid[j].root().parallel(k).vectorize(x, 4);
            outGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
            if (j == J-1) break;
            lPyramid[j].root().parallel(k).vectorize(x, 4);
            outLPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
        }
        break;
    case 3:
        // parallelize across yi instead of y: Bad idea - 1136, 889, 7144 
        output.split(y, y, yi, 8).parallel(yi);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().split(y, y, yi, 8).parallel(yi);
            gPyramid[j].root().parallel(k);
            outGPyramid[j].root().split(y, y, yi, 8).parallel(yi);
            if (j == J-1) break;
            lPyramid[j].root().parallel(k);
            outLPyramid[j].root().split(y, y, yi, 8).parallel(yi);
        }
        break;        
    case 4:
        // Parallelize, inlining all the laplacian pyramid levels
        // (they can be computed from the gaussian pyramids on the
        // fly): 491, 244, 4297
        output.split(y, y, yi, 32).parallel(y);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().split(y, y, yi, 4).parallel(y);
            gPyramid[j].root().parallel(k);
            outGPyramid[j].root().split(y, y, yi, 4).parallel(y);
        }
        break;                
    case 5:
        // Same as above with vectorization (now that we're doing more
        // math and less memory, maybe it will matter): 585, 204, 5389
        output.split(y, y, yi, 32).parallel(y).vectorize(x, 4);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
            gPyramid[j].root().parallel(k).vectorize(x, 4);
            outGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
        }
        break;
    case 6:
        // Also inline every other pyramid level: Bad idea - 2118, 562, 16873
        output.split(y, y, yi, 32).parallel(y).vectorize(x, 4);
        for (int j = 0; j < J; j+=2) {
            inGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
            gPyramid[j].root().parallel(k).vectorize(x, 4);
            outGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
        }
        break;
    case 7:
        // Take care of the boundary condition earlier to avoid costly
        // branching: 648, 242, 6037
        output.split(y, y, yi, 32).parallel(y).vectorize(x, 4);
        clamped.root().split(y, y, yi, 32).parallel(y).vectorize(x, 4);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
            gPyramid[j].root().parallel(k).vectorize(x, 4);
            outGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
        }
        break;
    case 8:
        // Unroll by a factor of two to try and simplify the
        // upsampling math: not worth it - 583, 297, 5716
        output.split(y, y, yi, 32).parallel(y).unroll(x, 2).unroll(yi, 2);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().split(y, y, yi, 4).parallel(y).unroll(x, 2).unroll(y, 2);
            gPyramid[j].root().parallel(k).unroll(x, 2).unroll(y, 2);
            outGPyramid[j].root().split(y, y, yi, 4).parallel(y).unroll(x, 2).unroll(y, 2);
        }
        break;                        
    case 9:
        // Same as case 5 but parallelize across y as well as k, in
        // case k is too small to saturate the machine: 693, 239, 5774
        output.split(y, y, yi, 32).parallel(y).vectorize(x, 4);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
            gPyramid[j].root().parallel(k).split(y, y, yi, 4).parallel(y).vectorize(x, 4);
            outGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
        }
        break;
    case 10:
        // Really-fine-grain parallelism. Don't both splitting
        // y. Should incur too much overhead to be good: 1083, 256, 5338
        output.parallel(y).vectorize(x, 4);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().parallel(y).vectorize(x, 4);
            gPyramid[j].root().parallel(k).parallel(y).vectorize(x, 4);
            outGPyramid[j].root().parallel(y).vectorize(x, 4);
        }
        break;      
    case 11:
        // Same as case 5, but don't vectorize above a certain pyramid
        // level to prevent boundaries expanding too much (computing
        // an 8x8 top pyramid level instead of e.g. 5x5 requires much
        // much more input). 602, 194, 4836
        output.split(y, y, yi, 32).parallel(y).vectorize(x, 4);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().parallel(y);
            gPyramid[j].root().parallel(k);
            outGPyramid[j].root().parallel(y);
            if (j < 5) {
                inGPyramid[j].vectorize(x, 4);
                gPyramid[j].vectorize(x, 4);
                outGPyramid[j].vectorize(x, 4);
            }
        }
        break;
    case 12:
        // The bottom pyramid level is gigantic. I wonder if we can
        // just compute those values on demand. Otherwise same as 5:
        // 293, 170, 5490
        output.split(y, y, yi, 32).parallel(y).vectorize(x, 4);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
            if (j > 0) gPyramid[j].root().parallel(k).vectorize(x, 4);
            outGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
        }
        break;
    case 13:
        // Should we inline the bottom pyramid level of everything?: 1044, 570, 17273
        output.split(y, y, yi, 32).parallel(y).vectorize(x, 4);
        for (int j = 1; j < J; j++) {
            inGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
            gPyramid[j].root().parallel(k).vectorize(x, 4);
            outGPyramid[j].root().split(y, y, yi, 4).parallel(y).vectorize(x, 4);
        }
        break;
    case 14:
        // 4 and 11 were pretty good for ARM. Can we do better by inlining
        // the root pyramid level like in 12? 427, 228, 4233
        output.split(y, y, yi, 32).parallel(y);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().parallel(y);
            if (j > 0) gPyramid[j].root().parallel(k);
            outGPyramid[j].root().parallel(y);          
        }
        break;

    case 100:
        // output stage only on GPU
        output.root().cudaTile(x, y, 32, 32);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root();
            gPyramid[j].root();
            outGPyramid[j].root();
            if (j == J-1) break;
            lPyramid[j].root();
            outLPyramid[j].root();
        }
        break;
    case 101:
        // all root on GPU, tiny blocks to prevent accidental bounds explosion
        output.root().cudaTile(x, y, 2, 2);
        for (int j = 0; j < J; j++) {
            inGPyramid[j].root().cudaTile(x, y, 2, 2);
            gPyramid[j].root().cudaTile(x, y, 2, 2);
            outGPyramid[j].root().cudaTile(x, y, 2, 2);
            if (j == J-1) break;
            lPyramid[j].root().cudaTile(x, y, 2, 2);
            outLPyramid[j].root().cudaTile(x, y, 2, 2);
        }
        break;
    case 102:
        // all root on GPU
        output.root().cudaTile(x, y, 32, 32); 
        for (int j = 0; j < J; j++) {
            int blockw = 32, blockh = 32;
            if (j > 3) {
                blockw = 2;
                blockh = 2;
            }
            inGPyramid[j].root().cudaTile(x, y, blockw, blockh);
            gPyramid[j].root().cudaTile(x, y, blockw, blockh);
            outGPyramid[j].root().cudaTile(x, y, blockw, blockh);
            if (j == J-1) break;
            lPyramid[j].root().cudaTile(x, y, blockw, blockh);
            outLPyramid[j].root().cudaTile(x, y, blockw, blockh);
        }
        break;
    case 103:
        // most root, but inline laplacian pyramid levels - 49ms on Tesla
        output.root().cudaTile(x, y, 32, 32);
        for (int j = 0; j < J; j++) {
            int blockw = 32, blockh = 32;
            if (j > 3) {
                blockw = 2;
                blockh = 2;
            }
            inGPyramid[j].root().cudaTile(x, y, blockw, blockh);
            gPyramid[j].root().cudaTile(x, y, blockw, blockh);
            outGPyramid[j].root().cudaTile(x, y, blockw, blockh);
        }
        break;
    default: 
        break;
    }

    printf("Success!\n");
    return 0;
}

