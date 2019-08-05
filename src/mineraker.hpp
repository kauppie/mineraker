#ifndef MINERAKER_HPP
#define MINERAKER_HPP

#include <cstddef> // std::size_t, std::ptrdiff_t
#include <exception>
#include <iostream>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

namespace rake {
using size_type = std::size_t;
using diff_type = std::ptrdiff_t;

bool init(Uint32 sdl_flags, int img_flags) {
  if (SDL_Init(sdl_flags) != 0) {
    std::cerr << "Error on SDL2 initialization: " << SDL_GetError();
    std::cin.get();
    return false;
  }
  if (IMG_Init(img_flags) != img_flags) {
    std::cerr << "Error on image initialization: " << IMG_GetError();
    std::cin.get();
    return false;
  }
  return true;
}

void quit() {
  IMG_Quit();
  SDL_Quit();
}

static const int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 480;

} // namespace rake

#endif