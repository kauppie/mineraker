#ifndef MINERAKER_HPP
#define MINERAKER_HPP

#include <cstddef> // std::size_t, std::ptrdiff_t
#include <exception>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

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
  if (TTF_Init() != 0) {
    std::cerr << "Error on tff initialization: " << SDL_GetError();
    std::cin.get();
    return false;
  }
  return true;
}

void quit() {
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

// Thread-safe registering single function call.
template <typename T> void call_once(T (*func)()) {
  static std::mutex map_mutex;
  std::lock_guard<std::mutex> lock(map_mutex);
  static std::unordered_map<decltype(func), bool> is_used_map;
  if (is_used_map.find(func) == is_used_map.end()) {
    func();
    is_used_map.insert({func, true});
  }
}

static constexpr const int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 480;
// Tile input texture dimensions used for clipping individual textures.
static constexpr const int TEXTURE_WIDTH_COUNT = 4, TEXTURE_HEIGHT_COUNT = 3;

} // namespace rake

#endif