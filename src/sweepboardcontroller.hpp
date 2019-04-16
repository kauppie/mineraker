#include "board_tile.hpp"
#include "sweepboard.hpp"

namespace msgn {

class SweepBoardController {
private:
  SweepBoard m_board;

public:
  SweepBoardController() : m_board(0,0,0) {}
  SweepBoardController(std::size_t width_, std::size_t height_,
                       double mine_fill_percent,
                       uint32_t seed = std::mt19937_64::default_seed)
      : m_board(width_, height_, mine_fill_percent, seed) {}
  ~SweepBoardController() {}

  void foo() {}

private:
}; // class SweepBoardController

} // namespace msgn