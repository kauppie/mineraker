#ifndef TEXT_HPP
#define TEXT_HPP

#include <exception>
#include <iostream>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "mineraker.hpp"
#include "renderable.hpp"
#include "texture.hpp"

namespace rake {

/**
 * Wrapper class for fonts and text rendering.
 */
class Text {
public:
  // @brief Default constructor.
  Text() : m_font(nullptr) {}

  // @brief Constructor which loads a font.
  Text(const std::string &path, int point_size) : m_font(nullptr) {
    load_font(path, point_size);
  }

  // @brief Deconstructor frees allocated resources.
  ~Text() noexcept { free(); }

  // @brief Loads font from specified path and using %point_size argument.
  void load_font(const std::string &path, int point_size) {
    free();
    m_font = TTF_OpenFont(path.c_str(), point_size);

    if (m_font == nullptr) {
      std::cerr << "\nError: Couldn't load font " << path;
    }
  }

  // @brief Loads font from specified path and using %point_size argument.
  // %index points to the type of font typeface to use -> italic, bold etc.
  void load_font(const std::string &path, int point_size, long index) {
    free();
    m_font = TTF_OpenFontIndex(path.c_str(), point_size, index);

    if (m_font == nullptr)
      throw std::runtime_error("Couldn't open file " + path);
  }

  // @brief Renders text aliased and blended to the background color.
  void render_solid() {
    m_texture_text.from_surface(
        TTF_RenderUTF8_Solid(m_font, m_text.c_str(), m_color));
  }

  // @brief Renders text anti-aliased and with specified background color box
  // surrounding the text.
  void render_shaded(SDL_Color bg_color) {
    m_texture_text.from_surface(
        TTF_RenderUTF8_Shaded(m_font, m_text.c_str(), m_color, bg_color));
  }

  // @brief Renders text anti-aliased and blended to the background color.
  void render_blended() {
    m_texture_text.from_surface(
        TTF_RenderUTF8_Blended(m_font, m_text.c_str(), m_color));
  }

  // @brief Frees allocated resources.
  void free() noexcept {
    if (m_font != nullptr) {
      TTF_CloseFont(m_font);
      m_font = nullptr;
    }
  }

private:
  TTF_Font *m_font;
  rake::Texture m_texture_text;
  std::string m_text;
  SDL_Color m_color;
};
} // namespace rake

#endif