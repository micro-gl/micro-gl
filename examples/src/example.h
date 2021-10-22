#pragma once

#include <iostream>
#include <chrono>
#include <SDL.h>

#include <cassert>
// Use (void) to silent unused warnings.
#define assertm(exp, msg) assert((msg, exp))

template<typename canvas_type, typename render_callback>
void example_run(canvas_type * canvas, int w, int h,
                 const render_callback & render,
                 int TEST_ITERATIONS=100,
                 bool custom_sdl=false) {
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Texture * texture;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Micro{gl} Example",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              w, h, 0);

    renderer = SDL_CreateRenderer(window, -1, 0);

    if(!custom_sdl)
        texture = SDL_CreateTexture(renderer,
    //                                SDL_PIXELFORMAT_RGB24,
                                    SDL_PIXELFORMAT_RGB888,
                                    SDL_TEXTUREACCESS_STATIC,
                                    w, h);

    auto render_test = [&](int N) -> int {
        auto ms = std::chrono::milliseconds(1);
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; ++i)
            render(window, renderer, texture);
        auto end = std::chrono::high_resolution_clock::now();
        auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return int_ms.count();
    };

    bool quit = false;
    SDL_Event event;

    int ms = render_test(TEST_ITERATIONS);
    std::cout << TEST_ITERATIONS << " iterations took: " << ms << "ms" << std::endl;

    while (!quit) {
        SDL_PollEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    quit = true;
                break;
        };

        render(window, renderer, texture);

        if(!custom_sdl) {
            SDL_UpdateTexture(texture, nullptr, canvas->pixels(),
                              canvas->width() * canvas->sizeofPixel());
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

template<typename canvas_type, typename render_callback>
void example_run(canvas_type * canvas,
                 const render_callback & render,
                 int TEST_ITERATIONS=100,
                 bool custom_sdl=false) {
    example_run<canvas_type, render_callback>(
            canvas, canvas->width(), canvas->height(), render, TEST_ITERATIONS, custom_sdl);
}

template<typename canvas_type, typename data_type, typename render_callback>
void example_run(canvas_type * canvas, data_type &data, int TEST_ITERATIONS,
                 const render_callback & render) {
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Texture * texture;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("SDL2 pixel_ Drawing",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              canvas->width(), canvas->height(), 0);

    renderer = SDL_CreateRenderer(window, -1, 0);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
                                SDL_TEXTUREACCESS_STATIC,
                                canvas->width(), canvas->height());

    auto render_test = [&](int N) -> int {
        auto ms = std::chrono::milliseconds(1);
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; ++i)
            render(data);
        auto end = std::chrono::high_resolution_clock::now();
        auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return int_ms.count();
    };

    bool quit = false;
    SDL_Event event;

    int ms = render_test(TEST_ITERATIONS);
    std::cout << ms << std::endl;

    while (!quit) {
        SDL_PollEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    quit = true;
                break;
        }

        render(data);

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(),
                          canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
