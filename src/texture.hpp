#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <fstream>
#include <string>

#include <SDL2/SDL.h>

#include "mineraker.hpp"

namespace rake {

class Texture {
private:
public:
  Texture() {}
  ~Texture() {}

  bool b_load_from_file(const std::string &path) {
    std::ifstream input_texture;
    input_texture.open(path);
  }

  void render() {}
};
} // namespace rake

#endif