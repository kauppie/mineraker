#include <chrono>
#include <iostream>
#include <thread>

#include <SDL2/SDL.h>

#include "../src/gamemanager.hpp"
#include "../src/mineboard.hpp"
#include "../src/mineboardsolver.hpp"
#include "../src/mineraker.hpp"
#include "../src/texture.hpp"
#include "../src/vectorspace.hpp"
#include "../src/windowmanager.hpp"

int main() {
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

  using namespace rake;
  MineBoard mb;
  MineBoardSolver mbs(mb);

  std::string set_mine = {"---1*23*2-2***334*4*3*2**323**"
                          "11-12*4*3-3*64****4*42334**5**"
                          "*2-1223*312*3*35*6422*11*33***"
                          "*2-1*113*4433113***2111112235*"
                          "22-23324****2124***3112323*22*"
                          "*1-1**3**5432*4***5*11***5*422"
                          "12246*5323*223**6*522124*5*6*2"
                          "-1*****435*31*434*6*311124***3"
                          "23335******213*44***3*211*345*"
                          "**1-3*75432224***66433*21111**"
                          "221-2***3333**46****35*412246*"
                          "11--25*5****54*5*643***3*4****"
                          "*2-12**4234*4*5***113*444***6*"
                          "*433*4*2-1224**33221334**4424*"
                          "4***222322*12*31--1*2**44*2-2*"
                          "***31-1**211111---1123*22*2-11"};
  mb.init(30, 16, 0, 170);
  mb.m_state = MineBoard::State::NEXT_MOVE;
  for (size_type i = 0; i < mb.m_tiles.size(); ++i) {
    if (set_mine[i] == '*')
      mb.m_tiles[i].set_mine();
    else
      mb.m_tiles[i].set_empty();
  }
  mb.m_set_numbered_tiles();
  mb.open_tile(0);
  // mbs.b_solve();

  rake::WindowManager wm{rake::SCREEN_WIDTH, rake::SCREEN_HEIGHT,
                         "Mineraker alpha",
                         SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE};
  rake::Texture tx(wm, "img/medium.png");
  rake::GameManager gm{&wm, &mb, &tx};

  SDL_Event event;
  SDL_DisplayMode display_mode;
  bool quit = false;
  SDL_SetRenderDrawColor(wm, 15, 40, 94, 255);

  SDL_GetWindowDisplayMode(wm, &display_mode);
  int refresh_rate = std::min(display_mode.refresh_rate, 24);

  // Use steady clock to avoid sensitivity to clock adjustments.
  auto frame_time = std::chrono::steady_clock::now().time_since_epoch();

  while (!quit) {
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT)
        quit = true;
      wm.handle_event(&event);
    }
    using std::chrono::duration, std::chrono::steady_clock;

    duration<double, std::nano> sleep_time =
        duration<double, std::micro>(1000000. / refresh_rate) - frame_time +
        steady_clock::now().time_since_epoch();
    std::this_thread::sleep_for(sleep_time);
    frame_time = steady_clock::now().time_since_epoch();

    SDL_RenderClear(wm);
    gm.render();
    SDL_RenderPresent(wm);

    if (mbs.b_overlap_solve() || mbs.b_common_solve() || mbs.b_pattern_solve())
      mbs.open_by_flagged();
    mbs.b_suffle_solve();
  }
}