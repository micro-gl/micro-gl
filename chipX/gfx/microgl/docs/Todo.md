#### GFX:
- Bezier curves with adaptive algorithm
- lines with stroke width via polygons ?
- draw lines with SDF
- draw lines with polygons
- rounded rectangle (check inigo sd functions)
- bilinear filtering for textures
- draw text
- filled polygon with AA
- draw masks
- draw shadows
- turn off AA for an EDGE (useful when rendering batch)
- do not raster outside the frame
- write a complete triangle rasterizer version with float points
- options for reciprocal LUT
- crashes when triangle is degenerate
- seperate classes for samplers, porter, blending, coders ?
- configurable bit resolution for mapper
- what to do with the quick mapper ?
- draw 3d meshes ? we can do it with the perspective correct texture mapper
- fill rules for adjescent triangles ?
- optimize the bilinear sampler (LUT ?)
- examine LUT for alpha compositing
- sub-pixel precision for lines
- clipping for lines
- DONE: fix bitmap width errors ==> width -1
- DONE: Filled Quadrilateral with AA
- DONE: textured Quadrilateral AA and Z correction
- DONE: round fixed points when sampling
- DONE: write nearest and bilinear sampler
- DONE: sub-pixel precision for circles
- DONE: sub-pixel precision for polygons

#### triangle raster
- try skipping lines
- try 8x8 block rasterizer - https://web.archive.org/web/20120625103536/http://devmaster.net/forums/topic/1145-advanced-rasterization/
- investigate the rasterizer further in terms of bits usages, I am almost there

#### gfx primitives
- draw circles with strokes (disks)
- draw capsules
- rounded rectangles