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

public:
  GameManager() {}
  ~GameManager() {}

private:
};
} // namespace msgn