#ifndef MINEBOARDSOLVER_HPP
#define MINEBOARDSOLVER_HPP

#include <algorithm>
#include <vector>

#include "boardtile.hpp"
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
  using difference_type = std::ptrdiff_t;

  MineBoard &m_board;

public:
  MineBoardSolver(MineBoard &board) : m_board(board) {}
  ~MineBoardSolver() noexcept {}

  // @brief Finds common neighbours between two tiles.
  static auto common_neighbours(MineBoard &board, size_type idx1,
                                     size_type idx2) {
    auto neighbrs1 = board.m_tile_neighbours_bnds(idx1);
    auto neighbrs2 = board.m_tile_neighbours_bnds(idx2);
    decltype(neighbrs1) common_nbrs;
    std::sort(neighbrs1.begin(), neighbrs1.end());
    std::sort(neighbrs2.begin(), neighbrs2.end());
    std::set_intersection(neighbrs1.begin(), neighbrs1.end(), neighbrs2.begin(),
                          neighbrs2.end(), std::back_inserter(common_nbrs));
    return common_nbrs;
  }

  static bool b_overlap_solve(MineBoard &board) {
    bool b_state_changed = false;
    auto &tiles = board.m_tiles;
    for (size_type idx = 0; idx < tiles.size(); ++idx) {
      if (tiles[idx].b_open && tiles[idx].is_number()) {
        auto neighbrs = board.m_tile_neighbours_bnds(idx);
        // Finds not opened neighbours of selected tile.
        auto nobrs = [&board, &neighbrs]() {
          std::vector<size_type> rvec;
          for (auto i : neighbrs)
            if (!board.m_tiles[i].b_open)
              rvec.emplace_back(i);
          return rvec;
        }();
        // Finds flagged neighbours of selected tile.
        auto flbrs = [&board, &neighbrs]() {
          std::vector<size_type> rvec;
          for (auto i : neighbrs)
            if (board.m_tiles[i].b_flagged)
              rvec.emplace_back(i);
          return rvec;
        }();
        // If tile's value equals the number of unopened neighbours, those
        // neighbours must be mines. If neighbour is unflagged, it will be
        // flagged.
        if (tiles[idx].tile_value == nobrs.size()) {
          for (auto nidx : nobrs) {
            if (!tiles[nidx].b_flagged) {
              tiles[nidx].set_flagged_unguarded();
              b_state_changed = true;
            }
          }
        }
        // If tile's value equals the number of flagged neighbours, all the
        // other neighbours must not be mines. If neighbour is closed, it will
        // be opened.
        else if (tiles[idx].tile_value == flbrs.size()) {
          for (auto nidx : flbrs) {
            if (!tiles[nidx].b_open) {
              tiles[nidx].set_open_unguarded();
              b_state_changed = true;
            }
          }
        }
      }
    }
    return b_state_changed;
  }

  static bool b_pattern_solve(MineBoard &board) { return false; }

  static bool b_suffle_solve(MineBoard &board) { return false; }

  static bool b_solve(MineBoard &board, size_type start_idx) {
    while (b_overlap_solve(board) || b_pattern_solve(board))
      ;
    return b_suffle_solve(board);
  }
}; // class MineBoardSolver

} // namespace rake

#endif