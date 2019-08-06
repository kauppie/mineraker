#include <chrono>
#include <iostream>

#include <SDL2/SDL.h>

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
                         "Mineraker, version .5",
                         SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE};
  rake::WindowManager wm1{rake::SCREEN_WIDTH, rake::SCREEN_HEIGHT, "copy",
                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE};
  rake::MineBoard mb{16, 16, 123};
  rake::GameManager gm{&wm, &mb};

  mb.init(50, 40);
  // gm.open_from(50);

  int mx = 0, my = 0;
  auto tilex = rake::SCREEN_WIDTH / mb.width(),
       tiley = rake::SCREEN_HEIGHT / mb.height();

  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT)
        quit = true;
      wm.handle_event(&e);
      wm1.handle_event(&e);
    }
    SDL_RenderClear(wm);
    gm.render();
    SDL_RenderPresent(wm);
  }
  return 0;
}