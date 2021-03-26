#include <iostream>
#include <microgl/tesselation/experiments/poly_heap_simplifier/qsort.h>

int compare_ints(const void* a, const void* b, void * ctx)
{
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;

    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;

    // return (arg1 > arg2) - (arg1 < arg2); // possible shortcut
    // return arg1 - arg2; // erroneous shortcut (fails if INT_MIN is present)
}
int main() {
    int vec[10] = {111,9,2,8,3,7,4,6,5,0};

    qsort_s(vec, 10, sizeof(int), compare_ints, nullptr);

    for (int i = 0; i < 10; i++) {
        printf("%d ", vec[i]);
    }
}

