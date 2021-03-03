#### some notes

- avoid branching in loops, it kills performance
- avoid non-inlined function calls inside loops, they kill performance
- virtual methods cannot be inlined - https://www.geeksforgeeks.org/inline-virtual-function/
- https://stackoverflow.com/questions/26115269/polymorphic-behaviour-without-virtual

- https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
- http://www.flipcode.com/archives/Faking_Polymorphism_with_C_Templates.shtml
- https://www.fluentcpp.com/2017/05/16/what-the-crtp-brings-to-code/
- https://eli.thegreenplace.net/2013/12/05/the-cost-of-dynamic-virtual-calls-vs-static-crtp-dispatch-in-c
- https://stackoverflow.com/questions/43569868/crtp-pattern-yet-store-inhomogenous-types-in-a-datastructure
- https://katyscode.wordpress.com/2013/08/22/c-polymorphic-cloning-and-the-crtp-curiously-recurring-template-pattern/
- https://eli.thegreenplace.net/2011/05/17/the-curiously-recurring-template-pattern-in-c



https://www.desmos.com/calculator/afoxmvnpk0

#### structure alignment
- every type's relative memory address should be a multiple of the number of it's bytes, that's what alignment means !!!
- https://software.intel.com/en-us/articles/coding-for-performance-data-alignment-and-structures


#### performance
```
storing and accessing 32bit pixels is faster than 8 bit values, much faster.
```

```
if we copy from_sampler same bitmap formats without blending/compositing, than it is
10% of the running with composting etc... so use it for optimization.

I witnessed 220 vs 25 fill rate
```

```
optimize normalized decoders of float points to use fixed point

```

```
always try to avoid multiplies in inner loops

```
- if porter-duff==none and blend-mode==none and alpha=1 then we can avoid blending completely,
this is a great optimization for texture mapping
- read this https://cellperformance.beyond3d.com/articles/2006/06/understanding-strict-aliasing.html
- make a ReadableBitmap interface

- curves sub division
https://github.com/pelson/antigrain/blob/master/agg-2.4/src/agg_curves.cpp

### size
- SDL : 87kb release, 124kb Debug

### tesselation
- http://www.normalesup.org/~cagne/internship_l3/mncubes_red/doc/html/bentley__ottmann_8cpp_source.html
- specialize tesselation algorithms for FPU as well

#### svg to canvas tech
- https://github.com/canvg/canvg
- https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths

#### svg stuff
- https://www.w3.org/TR/SVG/painting.html#FillProperties

#### fixed point motivation
- https://johnmcfarlane.github.io/cnl/
- https://www.touchgfx.com/documentation/html/structtouchgfx_1_1_c_w_r_util.html
- NICE:: https://www.keil.com/pack/doc/CMSIS/DSP/html/group__MatrixMult.html#ga2ec612a8c2c4916477fb9bc1ab548a6e

#### easing functions
- https://easings.net/en
- https://cubic-bezier.com/#1,0,0,1

#### paths
- https://github.com/google/skia/blob/master/src/gpu/GrTessellator.cpp
- https://www.w3.org/TR/2dcontext/#drawing-paths-to-the-canvas

#### complex decompose
- http://cis.usouthal.edu/~hain/general/Research.htm
- http://www.cis.southalabama.edu/~hain/general/Theses/Subramaniam_thesis.pdf
- https://github.com/mclaeysb/simplepolygon

#### bugs in simplifier
1. more than two components intersecetions - solved
2. two components that touch the same point
3. an edge that touches another edge
4. the algorithm does not handle degenerarte polygons
5. touching edges at endpoints are not considered intersections, unless
   they are part of a polygon definition

#### cool libs
- https://github.com/epezent/implot