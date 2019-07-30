#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include <memory>

#include "SDL2/SDL.h"

#include "mineboardcontroller.hpp"
#include "mineraker.hpp"
#include "windowmanager.hpp"

namespace rake {

class GameManager {
private:
  MineBoardController m_sbcontrol;
  std::unique_ptr<WindowManager> m_window;

public:
  GameManager() {}
  ~GameManager() {}
};

} // namespace rake

#endif