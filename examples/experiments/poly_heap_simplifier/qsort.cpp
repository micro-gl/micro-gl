#include <microgl/tesselation/experiments/poly_heap_simplifier/qsort.h>
#include "microgl/tesselation/triangles.h"

/*
 * A merge sort implementation, simplified from_sampler the qsort implementation
 * by Mike Haertel, which is a part of the GNU C Library.
 * Added context pointer, safety checks and return value.
 */

static void memcpy2(void *__a, const void *__b, size_t __c) {
    char *src = (char *) __b;
    char *dst = (char *) __a;
    for (int ix = 0; ix < __c; ++ix) {
        *(dst++) = *(src++);
    }
}

static void msort_with_tmp(void *b, size_t n, size_t s,
                           int (*cmp)(const void *, const void *, void *),
                           char *t, void *ctx)
{
    char *tmp;
    char *b1, *b2;
    size_t n1, n2;

    if (n <= 1)
        return;

    n1 = n / 2;
    n2 = n - n1;
    b1 = (char *)b;
    b2 = (char *)b + (n1 * s);

    msort_with_tmp(b1, n1, s, cmp, t, ctx);
    msort_with_tmp(b2, n2, s, cmp, t, ctx);

    tmp = t;

    while (n1 > 0 && n2 > 0) {
        if (cmp(b1, b2, ctx) <= 0) {
            memcpy2(tmp, b1, s);
            tmp += s;
            b1 += s;
            --n1;
        } else {
            memcpy2(tmp, b2, s);
            tmp += s;
            b2 += s;
            --n2;
        }
    }
    if (n1 > 0)
        memcpy2(tmp, b1, n1 * s);
    memcpy2(b, t, (n - n2) * s);
}

int qsort_s(void *b, size_t n, size_t s,
            int (*cmp)(const void *, const void *, void *), void *ctx)
{
    const size_t size = n*s;//st_mult(n, s);
    char buf[1024];

    if(s<2)
        return 0;
    if (!n)
        return 0;
    if (!b || !cmp)
        return -1;

    if (size < sizeof(buf)) {
        /* The temporary array fits on the small on-stack buffer. */
        msort_with_tmp(b, n, s, cmp, buf, ctx);
    } else {
        /* It's somewhat large, so malloc it.  */
//        char *tmp = malloc(size);
        char *tmp = new char[size];
        msort_with_tmp(b, n, s, cmp, tmp, ctx);
//        free(tmp);
        delete [] tmp;
    }
    return 0;
}

