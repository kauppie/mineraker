#ifndef MINEBOARDSOLVER_HPP
#define MINEBOARDSOLVER_HPP

#include <algorithm>
#include <limits>
#include <vector>

#include "boardtile.hpp"
#include "mineboard.hpp"
#include "mineraker.hpp"

namespace rake {

/**
 * @brief Class for solving MineBoards. This is separate class to avoid clutter
 * inside the MineBoard.
 */
class MineBoardSolver {
private:
  using this_type = MineBoardSolver;

  MineBoard &m_board;

public:
  MineBoardSolver(MineBoard &board) : m_board(board) {}
  MineBoardSolver(const this_type &other) : m_board(other.m_board) {} // test
  MineBoardSolver(this_type &&other) : m_board(other.m_board) {}      // test
  ~MineBoardSolver() noexcept {}

  // @brief Returns intersection or common neighbour indexes between the two
  // indexes. Expects vectors to be in sorted order.
  static auto common_idxs(const std::vector<size_type> &vec1,
                          const std::vector<size_type> &vec2) {
    std::vector<size_type> rv;
    std::set_intersection(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(),
                          std::back_inserter(rv));
    return rv;
  }

  // @brief Returns intersection or common neighbour positions between the two
  // positions. Expects vectors to be in sorted order.
  static auto common_poss(const std::vector<MineBoard::pos_type_t> &vec1,
                          const std::vector<MineBoard::pos_type_t> &vec2) {
    std::vector<MineBoard::pos_type_t> rv;
    std::set_intersection(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(),
                          std::back_inserter(rv),
                          MineBoard::pos_type_t::compare);
    return rv;
  }

  // @brief Finds common neighbours between two tiles using indexes.
  auto common_neighbours(size_type idx1, size_type idx2) {
    auto vec1 = m_board.m_tile_neighbours_bnds(idx1),
         vec2 = m_board.m_tile_neighbours_bnds(idx2);
    // %m_tiles_neighbours_bnds(size_type) returns unsorted data so it has to be
    // sorted.
    std::sort(vec1.begin(), vec1.end());
    std::sort(vec2.begin(), vec2.end());
    return common_idxs(vec1, vec2);
  }

  auto common_neighbours(MineBoard::pos_type_t pos1,
                         MineBoard::pos_type_t pos2) {
    // No need for sorting; %m_tiles_neighbours_bnds(pos_type_t) returns sorted
    // data.
    return common_poss(m_board.m_tile_neighbours_bnds(pos1),
                       m_board.m_tile_neighbours_bnds(pos2));
  }

  auto b_overlap_solve() {
    bool b_state_changed = false;
    auto &tiles = m_board.m_tiles;
    for (size_type idx = 0; idx < tiles.size(); ++idx) {
      if (tiles[idx].is_open() && tiles[idx].is_number()) {
        auto neighbrs = m_board.m_tile_neighbours_bnds(idx);
        std::vector<size_type> nobrs;
        std::vector<size_type> flbrs;
        for (auto i : neighbrs) {
          // Finds not opened neighbours of selected tile.
          if (!tiles[i].is_open())
            nobrs.emplace_back(i);
          // Finds flagged neighbours of selected tile.
          if (tiles[i].is_flagged())
            flbrs.emplace_back(i);
        }
        // If tile's value equals the number of unopened neighbours, those
        // neighbours must be mines. If neighbour is unflagged, it will be
        // flagged.
        if (tiles[idx].value() == nobrs.size()) {
          for (auto nidx : nobrs) {
            if (!tiles[nidx].is_flagged()) {
              tiles[nidx].set_flagged_unguarded();
              b_state_changed = true;
            }
          }
        }
        // If tile's value equals the number of flagged neighbours, all the
        // other neighbours must not be mines. If neighbour is closed, it will
        // be opened.
        else if (tiles[idx].value() == flbrs.size()) {
          for (auto nidx : neighbrs) {
            if (!(tiles[nidx].is_open() || tiles[nidx].is_flagged())) {
              // For performance reasons unguarded version of the set_open
              // method is used.
              tiles[nidx].set_open_unguarded();
              b_state_changed = true;
            }
          }
        }
      }
    }
    return b_state_changed;
  }

  auto b_pattern_solve() {
    bool b_state_changed = false;
    auto &tiles = m_board.m_tiles;
    for (size_type idx = 0; idx < tiles.size(); ++idx) {
      if (tiles[idx].is_open() && tiles[idx].is_number()) {
        auto neighbrs = m_board.m_tile_neighbours_bnds(idx);
        auto idx2 = std::numeric_limits<size_type>::max();
        for (auto n : neighbrs) {
          if (tiles[n].is_open() && tiles[n].value() == BoardTile::TILE_1) {
            // @todo For every 2, 1 tile pair.
          }
        }
        auto imin = std::min(idx, idx2), imax = std::max(idx, idx2);
        if (imax % m_board.width() != 0) {
          // @todo
        }
      }
    }
    return b_state_changed;
  }

  // @brief Solves from tiles which share neighbours. Opens Those tiles'
  // neighbours which CANNOT be mines.
  auto b_common_solve() { return false; }

  auto b_suffle_solve() { return false; }

  auto b_solve(size_type start_idx) {
    while (b_overlap_solve() || b_pattern_solve() || b_common_solve())
      ;
    return b_suffle_solve();
  }
};

} // namespace rake

#endif