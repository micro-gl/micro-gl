//
// Created by Tomer Shalev on 2019-06-29.
//

#include <stdio.h>
#include <iostream>

#include <string>
using namespace std;

 struct vec {
     uint8_t x;
     uint8_t y;
     uint8_t z;
}  ;

//typedef uint32_t RGB[3]  __attribute__((aligned(8)));
int N = 640*480*1;

uint32_t * buf_1 = new uint32_t[N] ;
vec * buf_2 = new vec[N];


void render_1() noexcept {

    for (int jx = 0; jx < 1000; ++jx) {

        for (int ix = 0; ix < N; ++ix) {
            buf_1[ix] =  5;
//            buf_2[ix].x =  ix;
//            buf_2[ix].y =  ix;
//            buf_2[ix].z =  ix;
        }

    }

}

int render_test(int M) {
    auto ms = std::chrono::milliseconds(1);
    auto start = std::chrono::high_resolution_clock::now();

//    for (int i = 0; i < M; ++i) {
        render_1();
//    }

    auto end = std::chrono::high_resolution_clock::now();

    return (end-start)/(ms*M);
}



int main() {

    int ms = render_test(1);

    std::cout << ms<<std::endl;



    return 0;

}