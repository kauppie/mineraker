#include <chrono>
#include <iostream>
#include <math.h>
#include <random>

#include <SDL2/SDL.h>

#include "gamemanager.hpp"
#include "mineboard.hpp"
#include "mineboardcontroller.hpp"
#include "mineboardformat.hpp"
#include "mineraker.hpp"
#include "windowmanager.hpp"

int main(int argc, char *argv[]) {

  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER |
               SDL_INIT_VIDEO) != 0) {
    std::cerr << "Error: " << SDL_GetError() << "\nPress any key to exit.";
    std::cin.get();
    return 1;
  }
  // SDL_Quit is called at program exit.
  atexit(SDL_Quit);

  bool quit = false;
  SDL_Event e;

  rake::WindowManager wm{800, 480, "test title"};

  SDL_SetRenderDrawColor(wm.renderer_ptr(), 255, 0, 0, 255);
  SDL_RenderClear(wm.renderer_ptr());
  SDL_RenderPresent(wm.renderer_ptr());

  std::cerr << wm.state;

  return 0;
}