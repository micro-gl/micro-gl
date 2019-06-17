//
// Created by Tomer Shalev on 2019-06-16.
//

#include <Functions.h>

float lerp(float a, float b, float t) {
    return (a * (1.0 - t)) + (b * t);
}

float smooth_step(float t) {
    return t*t*(3 - t*2);
}
