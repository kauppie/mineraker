#include <chrono>
#include <iostream>

#include <SDL2/SDL.h>

#include "gamemanager.hpp"
#include "mineboard.hpp"
#include "mineboardformat.hpp"
#include "mineraker.hpp"
#include "texture.hpp"
#include "windowmanager.hpp"

int main(int argc, char *argv[]) {

  if (!rake::init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER |
                      SDL_INIT_VIDEO,
                  IMG_INIT_PNG)) {
    std::cerr
        << "\nInitialization error; can't continue... Press any key to exit. ";
    std::cin.get();
    return 1;
  }
  // Register program-wide quit to be called on exit as SDL and IMG
  // initializations have been called.
  atexit(rake::quit);

  bool quit = false;
  SDL_Event e;

  rake::WindowManager wm{rake::SCREEN_WIDTH, rake::SCREEN_HEIGHT, "MINERAKER",
                         SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE};
  rake::MineBoard mb;
  mb.init(9, 9, time(0), 10);
  rake::Texture tx(wm, "img/medium.png");
  rake::GameManager gm{&wm, &mb, &tx};

  int mx = 0, my = 0;
  uint32_t m_button = 0;

  SDL_SetRenderDrawColor(wm, 15, 40, 94, 255);

  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT)
        quit = true;
      if (e.type == SDL_MOUSEBUTTONDOWN) {
        m_button = SDL_GetMouseState(&mx, &my);
        if (m_button & SDL_BUTTON(SDL_BUTTON_LEFT)) {
          gm.open_from(mx, my);
          std::cerr << "\nopen button";
        } else if (m_button & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
          gm.flag_from(mx, my);
          std::cerr << "\nflag button";
        }
      }
      wm.handle_event(&e);

      if (mb.state() == rake::MineBoard::State::GAME_WIN) {
        std::cerr << "\nGame WIN";
        mb.reset();
        mb.init(30, 30, time(0), 170);
      } else if (mb.state() == rake::MineBoard::State::GAME_LOSE) {
        std::cerr << "\nGame LOSE";
        mb.reset();
        mb.init(30, 30, time(0), 170);
      }
    }
    SDL_RenderClear(wm);
    gm.render();
    SDL_RenderPresent(wm);
  }
  return 0;
}