#include <chrono>
#include <iostream>
#include <thread>

#include <SDL2/SDL.h>

#include "gamemanager.hpp"
#include "mineboard.hpp"
#include "mineboardsolver.hpp"
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
  SDL_Event event;
  SDL_DisplayMode display_mode;

  int mx = 0, my = 0;
  uint32_t m_button = 0;

  rake::WindowManager wm{rake::SCREEN_WIDTH, rake::SCREEN_HEIGHT,
                         "Mineraker alpha",
                         SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE};
  rake::MineBoard mb;
  rake::Texture tx(wm, "img/medium.png");
  rake::GameManager gm{&wm, &mb, &tx};

  rake::MineBoardSolver mbs(mb);

  mb.init(30, 16, time(0), 99);

  SDL_GetWindowDisplayMode(wm, &display_mode);
  int refresh_rate = std::max(display_mode.refresh_rate, 60);

  // Use steady clock to avoid sensitivity to clock adjustments.
  auto frame_time = std::chrono::steady_clock::now().time_since_epoch();

  SDL_SetRenderDrawColor(wm, 15, 40, 94, 255);

  while (!quit) {
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT)
        quit = true;
      if (event.type == SDL_MOUSEBUTTONDOWN) {
        m_button = SDL_GetMouseState(&mx, &my);
        if (m_button & SDL_BUTTON(SDL_BUTTON_LEFT)) {
          gm.open_from(mx, my);
          std::cerr << "\nopen button";
        } else if (m_button & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
          gm.flag_from(mx, my);
          std::cerr << "\nflag button";
        }
      }
      wm.handle_event(&event);

      if (mb.state() == rake::MineBoard::State::GAME_WIN) {
        std::cerr << "\nGame WIN";
        mb.init(30, 16, time(0), 99);
        mbs.reset();
      } else if (mb.state() == rake::MineBoard::State::GAME_LOSE) {
        std::cerr << "\nGame LOSE";
        mb.init(30, 16, time(0), 99);
        mbs.reset();
      }
    }

    std::chrono::duration<int, std::nano> sleep_time =
        std::chrono::microseconds(1000000 / refresh_rate) - frame_time +
        std::chrono::steady_clock::now().time_since_epoch();
    std::this_thread::sleep_for(sleep_time);
    frame_time = std::chrono::steady_clock::now().time_since_epoch();

    SDL_RenderClear(wm);
    gm.render();
    SDL_RenderPresent(wm);

    if (mbs.b_overlap_solve())
      std::cerr << "\noverlap solve";
    if (mbs.b_common_solve())
      std::cerr << "\ncommon solve";
    if (mbs.b_pattern_solve())
      std::cerr << "\npattern solve";
    mbs.open_by_flagged();
  }
  return 0;
}