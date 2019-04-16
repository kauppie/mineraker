#ifndef SWEEPBOARDCONTROLLER_HPP
#define SWEEPBOARDCONTROLLER_HPP

#include "board_tile.hpp"
#include "sweepboard.hpp"

namespace msgn {

class SweepBoardController {
private:
  SweepBoard m_board;
  double m_mine_fill;
  std::mt19937_64::result_type m_seed;

public:
  SweepBoardController() {}
  SweepBoardController(
      std::size_t width, std::size_t height, double mine_fill,
      std::mt19937_64::result_type seed = std::mt19937_64::default_seed)
      : m_board(width, height, mine_fill, seed) {
    m_mine_fill = mine_fill;
    m_seed = seed;
  }
  ~SweepBoardController() {}

  void reset_board() {
    for (auto &tile : m_board.m_tiles)
      tile.reset();
  }

  void new_board() {
    m_board.m_zero_out();
    m_board.init(m_mine_fill, m_seed);
  }

private:
}; // class SweepBoardController

} // namespace msgn

#endif