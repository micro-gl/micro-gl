#### GFX:
- lines with stroke width via polygons ?
- draw lines with SDF
- rounded rectangle (check inigo sd functions)
- draw text
- draw masks
- draw shadows
- turn off AA for an EDGE (useful when rendering batch)
- crashes when triangle is degenerate
- what to do with the quick mapper ?
- draw 3d meshes ? we can do it with the perspective correct texture mapper
- fill rules for adjescent triangles ?
- examine LUT for alpha compositing
- CLIP RECT SUPPORT

#### triangle raster
- try skipping lines
- try 8x8 block rasterizer - https://web.archive.org/web/20120625103536/http://devmaster.net/forums/topic/1145-advanced-rasterization/
- investigate the rasterizer further in terms of bits usages, I am almost there

#### gfx primitives
- draw circles with strokes (disks)
- draw capsules
- rounded rectangles

#### transformations
- canvas transformations might be a good idea, especially for paths

#### data structures
- try to replace <vector> with https://lokiastari.com/ - this might take less space.

#### triangulation
http://www.personal.kent.edu/~rmuhamma/Compgeometry/MyCG/PolyPart/polyPartition.htm

#### code bloat
- vec2 is intantiated all over the place it seems
- array container virtual methods are included in every instance of template, consider throwing it away,
  I have seen a 6kb reduction in release mode which is a lot
- follow http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rt-abi

#### svg:
1. create hole puncher
2. create path maker
3. templatize arc_divider, path_tess, simplifier.
4. deal with holes AA bridge
5. use ear clipper from J. Orourke book
6. investigate solving degenrate cases some day.
