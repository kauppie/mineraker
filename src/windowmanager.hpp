#ifndef WINDOWMANAGER_HPP
#define WINDOWMANAGER_HPP

#include <string>

#include "SDL2/SDL.h"
#include "mineraker.hpp"

namespace rake {

class WindowManager {
 public:
  enum State { OK, NO_WINDOW, NO_RENDER_TARGET } state;
  // Initializing constructor.
  explicit WindowManager(int width, int height, const std::string& title,
                         uint32_t window_flags)
      : m_window(nullptr),
        m_renderer(nullptr),
        m_width(0),
        m_height(0),
        m_window_id(0),
        m_b_fullscreen(false) {
    init(width, height, title, window_flags);
  }

  explicit WindowManager()
      : m_window(nullptr),
        m_renderer(nullptr),
        m_width(0),
        m_height(0),
        m_window_id(0),
        m_b_fullscreen(false) {}

  ~WindowManager() { free(); }

  operator SDL_Window*() const { return m_window; }
  operator SDL_Renderer*() const { return m_renderer; }

  void init(int width, int height, const std::string& title,
            uint32_t window_flags) {
    reset();
    m_window =
        SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, width, height, window_flags);
    if (m_window == nullptr) {
      std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError()
                << "\n";
      state = NO_WINDOW;
    } else {
      m_title = title;
      m_width = width;
      m_height = height;
      m_window_id = SDL_GetWindowID(m_window);
      m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

      if (m_renderer == nullptr) {
        std::cerr << "Window renderer could not be created! SDL_Error: "
                  << SDL_GetError() << "\n";
        state = NO_RENDER_TARGET;
      } else
        state = OK;
    }
  }

  // Frees allocated resources and sets default values.
  void free() {
    if (m_renderer != nullptr) {
      SDL_DestroyRenderer(m_renderer);
      m_renderer = nullptr;
    }
    if (m_window != nullptr) {
      SDL_DestroyWindow(m_window);
      m_window = nullptr;
    }
  }

  // Sets all member variables to their default values.
  void reset() {
    free();
    m_title.clear();
    m_width = 0;
    m_height = 0;
    m_window_id = 0;
    m_b_fullscreen = false;
  }

  void handle_event(const SDL_Event* event) {
    if (event->type == SDL_WINDOWEVENT &&
        event->window.windowID == m_window_id) {
      switch (event->window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
          SDL_GetWindowSize(m_window, &m_width, &m_height);
          break;
        case SDL_WINDOWEVENT_CLOSE:
          free();
          break;
        default:
          break;
      }
    }
  }

  auto window() const { return m_window; }
  auto renderer() const { return m_renderer; }

  // Sets new title for the window.
  void title(const std::string& title) {
    SDL_SetWindowTitle(m_window, title.c_str());
    m_title = title;
  }
  // Returns title of the window.
  auto title() const { return m_title; }

  auto id() const { return m_window_id; }

  // Returns width of the window.
  auto width() const { return m_width; }

  // Returns height of the window.
  auto height() const { return m_height; }

  // Resizes window to specified pixel width and height.
  void resize(int width, int height) {
    SDL_SetWindowSize(m_window, width, height);
    m_width = width;
    m_height = height;
  }

  // Toggles window fullscreen. %desktop_resolution decides whether desktop
  // resolution is used for scaling.
  void toggle_fullscreen(bool desktop_resolution = true) {
    if (m_b_fullscreen) {
      SDL_SetWindowFullscreen(m_window, 0);
      m_b_fullscreen = false;
    } else {
      SDL_SetWindowFullscreen(m_window, desktop_resolution
                                            ? SDL_WINDOW_FULLSCREEN_DESKTOP
                                            : SDL_WINDOW_FULLSCREEN);
      m_b_fullscreen = true;
    }
  }

  // Set window maximized.
  void maximize() { SDL_MaximizeWindow(m_window); }

  // Set window minimized.
  void minimize() { SDL_MinimizeWindow(m_window); }

  // Reset window size to previous.
  void reset_size() { SDL_RestoreWindow(m_window); }

 private:
  SDL_Window* m_window;
  SDL_Renderer* m_renderer;
  std::string m_title;
  int m_width, m_height;
  uint32_t m_window_id;
  bool m_b_fullscreen;

};  // namespace rake

}  // namespace rake

#endif