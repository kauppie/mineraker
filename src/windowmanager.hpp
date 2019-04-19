#ifndef WINDOWMANAGER_HPP
#define WINDOWMANAGER_HPP

#include <string>
#include <string_view>

#include "SDL2/SDL.h"

namespace msgn {

class WindowManager {
public:
  using size_type = unsigned long;
  using diff_type = long;

private:
  SDL_Window *m_window;
  SDL_Renderer *m_renderer;
  SDL_Surface *m_surface;

  size_type m_width, m_height;
  volatile bool done;

public:
  WindowManager()
      : m_window(nullptr), m_renderer(nullptr), m_width(0), m_height(0) {}
  ~WindowManager() {
    if (m_renderer != nullptr)
      SDL_DestroyRenderer(m_renderer);
    if (m_window != nullptr)
      SDL_DestroyWindow(m_window);
  }

  void create_window(std::string_view title, diff_type width, diff_type height,
                     diff_type pos_x, diff_type pos_y, uint32_t flags) {
    m_window =
        SDL_CreateWindow(title.data(), pos_x, pos_y, width, height, flags);
    if (m_window == nullptr)
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation fail : %s\n",
                   SDL_GetError());
    m_surface = SDL_GetWindowSurface(m_window);
    m_renderer = SDL_CreateSoftwareRenderer(m_surface);
    if (m_renderer == nullptr)
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Render creation for surface fail : %s\n", SDL_GetError());
    SDL_SetRenderDrawColor(m_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(m_renderer);
  }

  operator SDL_Renderer*() {
    return m_renderer;
  }

private:
}; // class WindowManager

} // namespace msgn

#endif