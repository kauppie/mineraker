#ifndef MINEBOARDCONTROLLER_HPP
#define MINEBOARDCONTROLLER_HPP

#include <random>

#include "boardtile.hpp"
#include "mineboard.hpp"
#include "mineraker.hpp"

namespace rake {

class MineBoardController {
private:
  MineBoard m_board;

public:
  MineBoardController() {}
  MineBoardController(
      std::size_t width, std::size_t height, double mine_fill,
      std::mt19937_64::result_type seed = std::mt19937_64::default_seed)
      : m_board(width, height, mine_fill, seed) {}
  ~MineBoardController() noexcept {}

  void reset_board() {
    for (auto &tile : m_board.m_tiles)
      tile.reset();
  }

  void new_board(size_type no_mine_idx) {
    m_board.m_clear();
    m_board.init(no_mine_idx);
  }

  void resize_board(size_type width, size_type height) {
    m_board.set_dimensions(width, height);
  }

private:
};

} // namespace rake

#endif