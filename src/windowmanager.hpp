#include <string_view>
#include <string>

#include "SDL2/SDL.h"

namespace msgn {

class WindowManager {
public:
  using size_type = unsigned long;
  using diff_type = long;

private:
  SDL_Window *m_win;
  SDL_Renderer *m_rndrer;

  size_type m_width, m_height;

public:
  WindowManager() { SDL_Init(SDL_INIT_VIDEO); }
  ~WindowManager() {
    if (m_rndrer != nullptr)
      SDL_DestroyRenderer(m_rndrer);
    if (m_win != nullptr)
      SDL_DestroyWindow(m_win);
  }

  void create_window(std::string_view title, diff_type width, diff_type height,
                     diff_type pos_x, diff_type pos_y, uint32_t flags) {
    m_win = SDL_CreateWindow(title.data(), pos_x, pos_y, width, height, flags);
  }

private:
};

} // namespace msgn