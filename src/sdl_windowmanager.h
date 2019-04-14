#include <string_view>

#include "SDL2/SDL.h"

class SDLWindowManager {
public:
  using size_type = unsigned long;
  using diff_type = long;

private:
  SDL_Window *m_win;
  SDL_Renderer *renderer;

  size_type m_width, m_height;

public:
  SDLWindowManager() { SDL_Init(SDL_INIT_VIDEO); }
  ~SDLWindowManager() {
    if (m_win != nullptr)
      SDL_DestroyWindow(m_win);
  }

  void create_window(std::string_view title, diff_type width, diff_type height,
                     diff_type pos_x, diff_type pos_y, uint32_t flags) {
    m_win = SDL_CreateWindow(title.data(), pos_x, pos_y, width, height, flags);
  }

private:
};