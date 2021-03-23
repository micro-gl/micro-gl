#pragma once

//#include <cstddef>
using size_t = unsigned long;
extern
int qsort_s(void *b, size_t n, size_t s,
                int (*cmp)(const void *, const void *, void *), void *ctx);