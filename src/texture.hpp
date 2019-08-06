#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <fstream>
#include <iostream>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "mineraker.hpp"

namespace rake {

class Texture {
  using this_type = Texture;

public:
  Texture(const this_type &) = delete;
  Texture(this_type &&) = delete;

  // Constructor which uses %texture_from_file.
  explicit Texture(SDL_Renderer *renderer, const std::string &path)
      : m_texture(nullptr), m_surface(nullptr), m_width(0), m_height(0) {
    texture_from_file(renderer, path);
  }

  // Constructor which uses %surface_from_file.
  explicit Texture(const std::string &path)
      : m_texture(nullptr), m_surface(nullptr), m_width(0), m_height(0) {
    surface_from_file(path);
  }

  // Default constructor. Nullifies pointers.
  explicit Texture()
      : m_texture(nullptr), m_surface(nullptr), m_width(0), m_height(0) {}
  ~Texture() noexcept { free(); }

  operator auto() const { return m_texture; }
  operator auto() const { return m_surface; }

  // Returns whether image loading and texture assignment succesfully completed.
  // Assigns texture to specified renderer.
  bool texture_from_file(SDL_Renderer *renderer, const std::string &path) {
    free();
    m_texture = IMG_LoadTexture(renderer, path.c_str());
    if (m_texture == nullptr) {
      std::cerr << "Error: Couldn't load image " << path << " to texture\n"
                << SDL_GetError();
      return false;
    }
    SDL_QueryTexture(m_texture, nullptr, nullptr, &m_width, &m_height);
    return true;
  }

  // Returns whether image loading and surface assignment succesfully completed.
  bool surface_from_file(const std::string &path) {
    free();
    m_surface = IMG_Load(path.c_str());
    if (m_surface == nullptr) {
      std::cerr << "Error: Couldn't load image " << path << " to surface\n"
                << SDL_GetError();
      return false;
    }
    m_width = m_surface->w;
    m_height = m_surface->h;
    return true;
  }

  // Returns whether surface conversion to texture succesfully completed.
  bool texture_from_surface(SDL_Renderer *renderer) {
    m_free_texture();
    m_texture = SDL_CreateTextureFromSurface(renderer, m_surface);
    if (m_texture == nullptr) {
      std::cerr << "Error: Couldn't convert surface to texture\n"
                << SDL_GetError();
      return false;
    }
    return true;
  }

  // Returns texture pointer.
  auto texture() const noexcept { return m_texture; }

  // Returns surface pointer.
  auto surface() const noexcept { return m_surface; }

  // Renders the texture.
  void render(SDL_Renderer *renderer, int x, int y, SDL_Rect *clip = nullptr) {
    if (m_texture == nullptr) {
      if (m_surface == nullptr)
        return;
      texture_from_surface(renderer);
    }
    SDL_Rect render_area = {x, y, m_width, m_height};

    if (clip != nullptr) {
      render_area.w = clip->w;
      render_area.h = clip->h;
    }
    SDL_RenderCopy(renderer, m_texture, clip, &render_area);
  }

  // Renders the texture.
  void render(SDL_Renderer *renderer, SDL_Rect *src, SDL_Rect *dst) {
    if (m_texture == nullptr) {
      if (m_surface == nullptr)
        return;
      texture_from_surface(renderer);
    }
    SDL_RenderCopy(renderer, m_texture, src, dst);
  }

  // Destroys texture and frees surface allocated memory.
  void free() noexcept {
    m_free_texture();
    m_free_surface();
  }

  // Returns texture/surface width.
  auto width() const { return m_width; }

  // Returns texture/surface height.
  auto height() const { return m_height; }

  // Returns true if no image components are loaded. False otherwise.
  bool empty() const noexcept {
    return (m_texture == nullptr) && (m_surface == nullptr);
  }

private:
  // Frees texture used memory on the GPU.
  void m_free_texture() noexcept {
    if (m_texture != nullptr)
      SDL_DestroyTexture(m_texture);
    m_texture = nullptr;
  }

  // Frees surface used memory.
  void m_free_surface() noexcept {
    if (m_surface != nullptr)
      SDL_FreeSurface(m_surface);
    m_surface = nullptr;
  }

  SDL_Texture *m_texture;
  SDL_Surface *m_surface;
  int m_width, m_height;
};

} // namespace rake

#endif