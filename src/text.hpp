#ifndef TEXT_HPP
#define TEXT_HPP

#include <iostream>
#include <string>

#include <SDL2/SDL_ttf.h>

namespace rake {

/**
 * Wrapper class for fonts and text rendering.
 */
class Text {
public:
  Text() : m_font(nullptr) {}
  Text(const std::string &path, int point_size) : m_font(nullptr) {
    load_font(path, point_size);
  }
  ~Text() noexcept { free(); }

  void load_font(const std::string &path, int point_size) {
    free();
    m_font = TTF_OpenFont(path.c_str(), point_size);

    if (m_font == nullptr) {
      std::cerr << "\nError: Couldn't load font " << path;
    }
  }

  void load_font(const std::string &path, int point_size, long index) {
    free();
    m_font = TTF_OpenFontIndex(path.c_str(), point_size, index);

    if (m_font == nullptr) {
      std::cerr << "\nError: Couldn't load font " << path;
    }
  }

  void free() noexcept {
    if (m_font != nullptr) {
      TTF_CloseFont(m_font);
      m_font = nullptr;
    }
  }

private:
  void m_set_default() { m_font = nullptr; }

  TTF_Font *m_font;
};
} // namespace rake

#endif