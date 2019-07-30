#ifndef WINDOWMANAGER_HPP
#define WINDOWMANAGER_HPP

#include <string>

#include "SDL2/SDL.h"

#include "mineraker.hpp"

namespace rake {

class WindowManager {
public:
  enum State { OK, NO_WINDOW, NO_RENDER_TARGET } state;
  // Default constructor.
  WindowManager(int32_t width, int32_t height, const std::string &title)
      : m_window(nullptr), m_renderer(nullptr), m_width(width),
        m_height(height), m_title(title) {
    m_window = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, m_width, m_height,
                                SDL_WINDOW_SHOWN);
    if (m_window == nullptr) {
      std::cerr << "\nWindow could not be created! SDL_Error: "
                << SDL_GetError();
      state = NO_WINDOW;

    } else {
      m_renderer = SDL_CreateRenderer(m_window, -1, 0);
      if (m_renderer == nullptr) {
        std::cerr << "\nWindow renderer could not be created! SDL_Error: "
                  << SDL_GetError();
        state = NO_RENDER_TARGET;
      } else
        state = OK;
    }
  }

  // Default destructor.
  ~WindowManager() {
    if (m_renderer != nullptr)
      SDL_DestroyRenderer(m_renderer);
    if (m_window != nullptr)
      SDL_DestroyWindow(m_window);
  }

  auto window_ptr() const { return m_window; }
  auto renderer_ptr() const { return m_renderer; }

  // Returns old title and sets new title.
  auto title(const std::string &title) {
    SDL_SetWindowTitle(m_window, title.c_str());
    const auto &old = m_title;
    m_title = title;
    return old;
  }
  // Returns title of the window.
  auto title() const { return m_title; }
  // Returns old width and sets new width.
  auto width(int32_t width) {
    SDL_SetWindowSize(m_window, width, m_height);
    auto old = m_width;
    m_width = width;
    return old;
  }
  // Returns width of the window.
  auto width() const { return m_width; }
  // Returns old height and sets new height.
  auto height(int32_t height) {
    SDL_SetWindowSize(m_window, m_width, height);
    auto old = m_height;
    m_height = height;
    return old;
  }
  // Returns height of the window.
  auto height() const { return m_height; }

  void resize(int32_t width, int32_t height) {
    SDL_SetWindowSize(m_window, width, height);
    m_width = width;
    m_height = height;
  }

private:
  SDL_Window *m_window;
  SDL_Renderer *m_renderer;
  std::string m_title;
  int32_t m_width, m_height;
}; // namespace rake

} // namespace rake

#endif