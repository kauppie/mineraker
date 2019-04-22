#ifndef MINEBOARDSOLVER_H
#define MINEBOARDSOLVER_H

#include <vector>

#include "mineboard.hpp"

namespace rake {

/**
 * @brief Class for solving MineBoards. This is separate class to avoid clutter
 * inside the MineBoard.
 */
class MineBoardSolver {
private:
  using this_type = MineBoardSolver;
  using size_type = MineBoard::size_type;

  MineBoard control;

public:
  MineBoardSolver(const MineBoard &board) : control(board) {}
  ~MineBoardSolver() noexcept {}

  bool b_solvable(const MineBoard &board, size_type start_idx) {
    auto idx = start_idx;
    auto &tiles = control.m_tiles;
    auto &tile = tiles[idx];
    if (!tile.is_empty())
      return false;

    auto mine_neighbours_count = [&](size_type i) {
      size_type ret = 0;
      for (auto j : control.m_tile_neighbours_bnds(i))
        if (tiles[j].is_mine())
          ++ret;
      return ret;
    };

    auto not_opened_neighbours = [&](size_type i) {
      std::vector<size_type> ret;
      for (auto j : control.m_tile_neighbours_bnds(i))
        if (!tiles[j].b_open)
          ret.emplace_back(j);
      return ret;
    };

    auto not_opened_flagged_neighbours = [&](size_type i) {
      std::vector<size_type> ret;
      for (auto j : control.m_tile_neighbours_bnds(i))
        if (!tiles[j].b_open && !tiles[j].b_flagged)
          ret.emplace_back(j);
      return ret;
    };

    for (auto i = 0ull; i < tiles.size(); ++i) {
      if (tiles[i].is_number()) {
        auto nneighbours = not_opened_neighbours(i);
        if (tiles[i].tile_value == nneighbours.size())
          for (auto n : nneighbours)
            tiles[n].set_flagged();
      }
    }
  }
}; // class MineBoardSolver

} // namespace rake

#endif