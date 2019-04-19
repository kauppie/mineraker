#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include "SDL2/SDL.h"

#include "sweepboardcontroller.hpp"
#include "windowmanager.hpp"

namespace msgn {

class GameManager {
public:
  using size_type = std::size_t;

private:
  SweepBoardController m_sbcontrol;
  WindowManager m_window;

  bool m_exit;

public:
  GameManager() : m_exit(false) {
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
      m_exit = true;
    }
    m_window.create_window("test", 400, 320, 100, 100, 0);
  }
  ~GameManager() noexcept {}

  void loop() {
    while (!m_exit) {
      if (m_handle_events())
        return;
      DrawChessBoard(m_window);
    }
  }

  static void DrawChessBoard(SDL_Renderer *renderer) {
    int row = 0, column = 0, x = 0;
    SDL_Rect rect, darea;

    /* Get the Size of drawing surface */
    SDL_RenderGetViewport(renderer, &darea);

    for (; row < 8; row++) {
      column = row % 2;
      x = column;
      for (; column < 4 + (row % 2); column++) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);

        rect.w = darea.w / 8;
        rect.h = darea.h / 8;
        rect.x = x * rect.w;
        rect.y = row * rect.h;
        x = x + 2;
        SDL_RenderFillRect(renderer, &rect);
      }
    }
  }

private:
  bool m_handle_events() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        m_exit = true;
        return true;
      }

      if ((e.type == SDL_KEYDOWN) && (e.key.keysym.sym == SDLK_ESCAPE)) {
        m_exit = true;
        return true;
      }
    }
    return m_exit;
  }

}; // class Gamemanager

} // namespace msgn

#endif