#include <chrono>
#include <iostream>
#include <random>

#include "SDL2/SDL.h"

#include "gamemanager.hpp"
#include "sweepboard.hpp"
#include "sweepboardformat.hpp"
#include "sweepboardcontroller.hpp"
#include "windowmanager.hpp"

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO);
  msgn::WindowManager wm;
  wm.create_window("test", 480,320,100,100,0);

  return 0;
}