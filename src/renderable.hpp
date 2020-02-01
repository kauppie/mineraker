#ifndef TEXTURED_TARGER_HPP
#define TEXTURED_TARGER_HPP

#include <SDL2/SDL.h>

namespace rake {

/**
 * Class meant to be inherited with common functionality as renderable.
 * Whether this helper class is even usefull is to be realised.
 */
class Renderable {
public:
  explicit Renderable() : m_texture(nullptr) {}
  ~Renderable() noexcept { free_texture(); }

  operator SDL_Texture*() const { return m_texture; }

  auto texture() const { return m_texture; }

  void texture(SDL_Texture* texture) {
    if (texture != nullptr) {
      free_texture();
      m_texture = texture;
    }
  }

  void free_texture() const noexcept {
    if (m_texture != nullptr)
      SDL_DestroyTexture(m_texture);
  }

  virtual void render() = 0;

protected:
  SDL_Texture* m_texture;
};

} // namespace rake

#endif