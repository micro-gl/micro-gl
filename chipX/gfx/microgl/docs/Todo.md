#### GFX:
- write nearest and bilinear sampler
- Bezier curves with adaptive algorithm
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
- DONE: fix bitmap width errors ==> width -1
- DONE: Filled Quadrilateral with AA
- DONE: textured Quadrilateral AA and Z correction
- DONE: round fixed points when sampling

#### triangle raster
- try skipping lines
- try 8x8 block rasterizer - https://web.archive.org/web/20120625103536/http://devmaster.net/forums/topic/1145-advanced-rasterization/