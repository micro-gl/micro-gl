#pragma once

#include "model_3d.h"
#include "microgl/tesselation/triangles.h"

template <typename number>
model_3d<number> cube_3d =
        {
        12, microtess::triangles::indices::TRIANGLES,
        { // vertices
            { -1, -1, 1 }, { 1, -1, 1}, { 1, -1, -1  }, { -1, -1, -1 }, // bottom wall
            { -1,  1, -1}, { 1, 1, -1  }, { 1, 1,  1 }, { -1, 1, 1   },  // top wall
            { -1,  -1, -1}, { 1, -1, -1  }, { 1, 1, -1   }, { -1, 1,  -1 }, // far wall
            { -1,  1, 1}, { 1, 1, 1  }, { 1, -1, 1   }, { -1, -1,  1 }, // near wall
            { -1,  1, -1}, { -1, 1, 1  }, { -1, -1, 1   }, { -1, -1,  -1 }, // left wall
            { 1,  1, 1}, { 1, 1, -1  }, { 1, -1, -1   }, { 1, -1,  1 }, // right wall
        },
        {
                {0,1},{1,1},{1,0},{0,0},
                {0,1},{1,1},{1,0},{0,0},
                {0,1},{1,1},{1,0},{0,0},
                {0,1},{1,1},{1,0},{0,0},
                {0,1},{1,1},{1,0},{0,0},
                {0,1},{1,1},{1,0},{0,0},
        },
        { // indices
            // bottom wall
            0,1,2,
            2,3,0,
            // top wall
            4,5,6,
            6,7,4,
            // far wall
            8,9,10,
            10,11,8,
            // near wall
            12,13,14,
            14,15,12,
            // left wall
            16,17,18,
            18,19,16,
            // right wall
            20,21,22,
            22,23,20

        }
};
