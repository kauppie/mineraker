#include <array>
#include <chrono>
#include <iostream>
#include <math.h>
#include <random>

#include <SDL2/SDL.h>

#include <boost/locale.hpp>

#include "gamemanager.hpp"
#include "mineboard.hpp"
#include "mineboardcontroller.hpp"
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

  rake::WindowManager wm{rake::SCREEN_WIDTH, rake::SCREEN_HEIGHT,
                         "Mineraker, version .5"};
  rake::MineBoard mb{16, 16, 691548};
  rake::GameManager gm{&wm, &mb};

  mb.init(50, 40);
  gm.open_from(50);

  int mx = 0, my = 0;
  bool mouseDown;
  auto tilex = rake::SCREEN_WIDTH / mb.width(),
       tiley = rake::SCREEN_HEIGHT / mb.height();

  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT)
        quit = true;
      if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN ||
          e.type == SDL_MOUSEBUTTONUP) {
        SDL_GetMouseState(&mx, &my);

        if (e.type == SDL_MOUSEBUTTONDOWN)
          mouseDown = false;
        else if (e.type == SDL_MOUSEBUTTONUP)
          mouseDown = true;
      }
    }

    if (mouseDown) {
      mouseDown = false;
      auto idx = (my / tiley) * mb.width() + (mx / tilex);
      std::cerr << "\n" << idx;
      gm.open_from(idx);
    }

    SDL_RenderClear(wm);
    gm.render();
    SDL_RenderPresent(wm);
  }
  return 0;
}