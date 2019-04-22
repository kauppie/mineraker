#include <chrono>
#include <iostream>
#include <random>

//#include "SDL2/SDL.h"

#include "gamemanager.hpp"
#include "mineboard.hpp"
#include "mineboardcontroller.hpp"
#include "mineboardformat.hpp"
#include "windowmanager.hpp"

int main(int argc, char *argv[]) {
  rake::MineBoard mb{71, 34, 481, 489648956};

  std::cout << mb.seed() << "\n";
  return 0;
}