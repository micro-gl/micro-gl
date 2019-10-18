#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/PixelCoder.h>
#include <microgl/tesselation/simple_components_tree.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;
//extern template class chunker<microgl::vec2_f>;

typedef Canvas<uint32_t, RGB888_PACKED_32> Canvas24Bit_Packed32;

Canvas24Bit_Packed32 * canvas;

void loop();
void init_sdl(int width, int height);

using namespace tessellation;
using index_t = unsigned int;

float t = 0;


chunker<vec2_f> poly_degenerate_hole() {
    chunker<vec2_f> A;

    A.push_back_and_cut({
            {100,100},
            {300,100},
            {300,300},
            {100,300},

            {100,100},
            {100,200},
            {200,200},

//            {220,200},
//            {220,250},
//            {250,250},
//            {250,200},
//            {220,200},

//            {200,200},
            {200,100},
//            {100,100},
//
//            {100,300},

    });

    return A;
}


dynamic_array<vec2_f> box(float left, float top, float right, float bottom, bool ccw=false) {
    if(!ccw)
        return {
                {left,top},
                {right,top},
                {right,bottom},
                {left,bottom},
        };

    return{
            {left,top},
            {left,bottom},
            {right,bottom},
            {right,top},
    };
};

chunker<vec2_f> poly_degenerate_multipepoints() {
    chunker<vec2_f> A;

    A.push_back_and_cut({
            {100,100},
            {100,100},
            {100,100},
            {100,100},

            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},
            {300,100},

            {300,300},
            {300,300},
            {300,300},
            {300,300},

            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
            {100,300},
    });


    return A;
}

chunker<vec2_f> poly_inter_1() {
    chunker<vec2_f> A;

    A.push_back_and_cut({
                                {100,100},
                                {400,400},
                                {100,400},
                                {400,100},
                        });

    return A;
}

chunker<vec2_f> poly_case_touches_1() {
    chunker<vec2_f> A;

    A.push_back_and_cut({
                                {100,100},
                                {100,400},
                                {400,400},
                                {400,100},
                                {250,250}
                        });

    A.push_back_and_cut({
                                {100,100},
                                {250,250},
                                {400,100},
                        });
//
//    A.push_back_and_cut({
//                                {100,100},
//                                {100,400},
//                                {250,250},
//                        });
//
//    A.push_back_and_cut({
//                                {0,0},
//                                {500,0},
//                                {500,500},
//                                {0,500},
//                        });

    return A;
}

chunker<vec2_f> poly_tag_merge_test() {
    chunker<vec2_f> A;

    A.push_back_and_cut(box(0,0,400,400));
    A.push_back_and_cut(box(20,20,400-20,400-20));
//    A.push_back_and_cut(box(40,40,400-40,400-40));

    A.push_back_and_cut(box(60,60,400-60,400-60, true));
    A.push_back_and_cut(box(80,80,400-80,400-80, true));
//    A.push_back_and_cut(box(100,100,400-100,400-100, true));

    A.push_back_and_cut(box(100,100,400-100,400-100,true));

    return A;
}

chunker<vec2_f> poly_inter_2() {

    chunker<vec2_f> A;

    A.push_back_and_cut({
            {0.0,0.0},
            {50,0},
            {50,50},
            {0,50},
    });

    A.push_back_and_cut({
        {50+0.0,50+0.0},
        {50+50,0+50},
        {50+50,50+50},
        {0+50,50+50}
    });

    return A;
}

chunker<vec2_f> poly_inter_side() {
    dynamic_array<index_t> locations;

    chunker<vec2_f> A{
            {0.0,0.0},
            {400,0},
            {400,400},
            {0,400},
    };

    A.cut_chunk();

    A.push_back_and_cut({
            {0.0,0.0},
            {400,0},
            {400,400},
//            {0,400},
    });

    return A;
}

chunker<vec2_f> poly_inter_nested_3() {

    chunker<vec2_f> A;

    A.push_back_and_cut({
            {0.0,0.0},
            {300,0},
            {300,300},
            {0,300},
    });

    A.push_back_and_cut({
        {0,50 + 0.0},
        {300 + 50, 0 + 50},
        {300 + 50, 300 - 50},
        {0,300 - 50}
    });

    A.push_back_and_cut({
        {100+0.0,100+0.0},
        {300+100,0+100},
        {300+100,300-100},
        {0+100,300-100},
    });

    A.push_back_and_cut({
            {10,150},
            {500,150},
            {500,170},
            {10,170},
    });

    return A;
}

chunker<vec2_f> poly_inter_nested_2() {

    chunker<vec2_f> A;

    A.push_back_and_cut({
            {0,50.},
            {250,50},
            {250,250},
            {0,250},
    });

    A.push_back_and_cut({
            {100,100},
            {400,100},
            {400,200},
            {100,200},
    });

    A.push_back_and_cut({
            {10,150},
            {300,150},
            {300,170},
            {10,170},
    });

    return A;
}


chunker<vec2_f> poly_inter_weird_touch() {
    chunker<vec2_f> A, B;

    A.push_back_and_cut({
            {50.0,50.0},
            {300,50},
            {300,350},
            {100,350},

            {100,50},
    });

    B.push_back_and_cut({
            {50.0,350.0},
            {300,350},
            {300,50},
            {100,50},

            {100,350},
    });

    return A;
}


chunker<vec2_f> poly_inter_nested_disjoint() {
    chunker<vec2_f> A;

    A.push_back_and_cut({
            {0.0,0.0},
            {300,0},
            {300,300},
            {0,300},
    });

    A.push_back_and_cut({
            {50,50.},
            {50,250},
            {400,250},
            {400,50},
    });

    A.push_back_and_cut({
            {350,100.},
            {500,100},
            {500,150},
            {350,150},
    });

    return A;
}

chunker<vec2_f> poly_inter_deg() {
    chunker<vec2_f> A;

    A.push_back_and_cut({
            {400,400},
            {0,400},
            {0,0},
            {400,0},

            {50,350},
            {100,350},
            {100,100},

    });

    return A;
}

chunker<vec2_f> poly_inter_star() {
    chunker<vec2_f> A;

    A.push_back_and_cut({
                                {150, 150},
                                {450,150},
                                {200,450},
                                {300,50},
                                {400,450},
                        });

    return A;
}

chunker<vec2_f> poly_double() {
    chunker<vec2_f> A;

//    A.push_back_and_cut(box(0,0,300,300));
//    A.push_back_and_cut({
//                                {50,0},
//                                {300,0},
//                                {300,300},
//                        });

//    A.push_back_and_cut({
//                                {300,300},
//                                {200,150},
//                                {150,150},
//                        });
//

    A.push_back_and_cut(box(100,0,300,200));
    A.push_back_and_cut(box(200,0,400,200));

    return A;
}

// todo:: directions of degenrate polygons is incorrect

template <typename T>
void render_polygon(chunker<T> pieces);

void render() {
    t+=.05f;
//    std::cout << t << std::endl;

//    render_polygon(poly_degenerate_hole());
//
//    render_polygon(poly_degenerate_multipepoints());

//    render_polygon(poly_inter_1());
//
//    render_polygon(poly_inter_weird_touch());

//    render_polygon(poly_inter_nested_3());
//    render_polygon(poly_inter_nested_disjoint());
//    render_polygon(poly_inter_nested_2());

//    render_polygon(poly_inter_2());

//    render_polygon(poly_inter_side());

//    render_polygon(poly_inter_deg());

//    render_polygon(poly_double());
//
//    render_polygon(poly_inter_star());
    render_polygon(poly_inter_nested_3());
//    render_polygon(poly_inter_1());
//    render_polygon(poly_case_touches_1());

//    render_polygon(poly_tag_merge_test());

}


// debug - 354kb, O3 - 64kb, Os - 44kb
// debug - 340kb, O3 - 60kb, Os - 40kb
// debug - 332kb, O3 - 56kb, Os - 36kb
// debug - 323kb, O3 - 56kb, Os - 36kb
// debug - 328kb, O3 - 56kb, Os - 36kb // templatized simplify_comps
// debug - 332kb, O3 - 56kb, Os - 36kb // templatized comps_tree

template <typename T>
void render_polygon(chunker<T> pieces) {
    using index = unsigned int;

    canvas->clear(WHITE);
    using sct = tessellation::simple_components_tree<float>;

    sct::tree tree;
    sct::compute(
            pieces,
            tree);

//    std::cout<<tree.nodes->index_poly;
    for (index ix = 0; ix < tree.pieces.size(); ++ix) {
        auto chunk = tree.pieces[ix];
        std::cout << "chunk: " << chunk.size << std::endl;

//                if(ix!=2)
//                    continue;

//        canvas->drawQuad(RED, 0, 0, 100,100, 0,255);
        canvas->drawPolygon<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
                chunk.data,
                chunk.size,
                120,
                polygons::hints::SIMPLE
        );

    }

}



int main() {
    init_sdl(W, H);
    loop();
}

void init_sdl(int width, int height) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("SDL2 Pixel Drawing", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
            SDL_TEXTUREACCESS_STATIC, width, height);

    canvas = new Canvas24Bit_Packed32(width, height, new RGB888_PACKED_32());

}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; ++i) {
        render();
    }

    auto end = std::chrono::high_resolution_clock::now();
    return (end-start)/(ms*N);
}

void loop() {
    bool quit = false;
    SDL_Event event;

    // 100 Quads
    int ms = render_test(TEST_ITERATIONS);
    std::cout << ms << std::endl;

    while (!quit)
    {
        SDL_PollEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYUP:
                if( event.key.keysym.sym == SDLK_ESCAPE )
                    quit = true;
                break;
        }
//
//        render();

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(),
                canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
