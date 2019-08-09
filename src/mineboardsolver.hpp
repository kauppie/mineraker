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

  // Keep vector of checked numbered tiles for %open_by_flagged to not bother
  // with already checked ones.
  std::vector<bool> m_checked_number_tiles;

public:
  MineBoardSolver(MineBoard &board) : m_board(board) {}
  MineBoardSolver(const this_type &other)
      : m_board(other.m_board),
        m_checked_number_tiles(other.m_checked_number_tiles) {}
  MineBoardSolver(this_type &&) = delete;
  ~MineBoardSolver() noexcept {}

  void reset() {
    for (auto checked : m_checked_number_tiles)
      checked = false;
  }

  auto flagged_neighbours_count(size_type idx) const {
    size_type count = 0;
    auto neighbrs = m_board.m_tile_neighbours_bnds(idx);
    for (auto i : neighbrs)
      if (m_board.m_tiles[i].is_flagged())
        ++count;
    return count;
  }

  auto open_neighbours_count(size_type idx) const {
    size_type count = 0;
    auto neighbrs = m_board.m_tile_neighbours_bnds(idx);
    for (auto i : neighbrs)
      if (m_board.m_tiles[i].is_open())
        ++count;
    return count;
  }

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
  static auto common_poss(const std::vector<MineBoard::pos_type> &vec1,
                          const std::vector<MineBoard::pos_type> &vec2) {
    std::vector<MineBoard::pos_type> rv;
    std::set_intersection(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(),
                          std::back_inserter(rv), MineBoard::compare);
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

  auto common_neighbours(MineBoard::pos_type pos1, MineBoard::pos_type pos2) {
    // No need for sorting; %m_tiles_neighbours_bnds(pos_type) returns sorted
    // data.
    return common_poss(m_board.m_tile_neighbours_bnds(pos1),
                       m_board.m_tile_neighbours_bnds(pos2));
  }

  // @brief Determine which tiles to open based on the amount of flagged tiles
  // surrounding them and their value. Optimized to an extent that it won't
  // calculate on tiles which have their neighbours either opened or flagged.
  // These are stored in a boolean vector.
  bool open_by_flagged() {
    bool b_state_changed = false;
    // To make things prettier.
    auto &tiles = m_board.m_tiles;
    if (m_checked_number_tiles.size() != tiles.size())
      m_checked_number_tiles.resize(tiles.size(), false);

    for (size_type i = 0; i < tiles.size(); ++i) {
      if (tiles[i].is_open() && tiles[i].is_number()) {
        auto flagged_neighbrs_count = flagged_neighbours_count(i);
        // If the amount of flagged tiles equals the value representing mine
        // count on the current tile, open those which are not flagged.
        if (!m_checked_number_tiles[i] &&
            tiles[i].value() == flagged_neighbrs_count) {
          m_board.m_on_next_move(i);
          if (open_neighbours_count(i) + flagged_neighbrs_count ==
              m_board.m_neighbour_count(i))
            m_checked_number_tiles[i] = true;
          b_state_changed = true;
        }
      }
    }
    return b_state_changed;
  }

  // @brief Compares each tile's value to it's unopened neighbour tile count and
  // flag those if they are equal.
  auto b_overlap_solve() {
    bool b_state_changed = false;
    auto &tiles = m_board.m_tiles;
    for (size_type idx = 0; idx < tiles.size(); ++idx) {
      if (tiles[idx].is_open() && tiles[idx].is_number()) {
        auto neighbrs = m_board.m_tile_neighbours_bnds(idx);
        std::vector<size_type> nobrs;
        // Finds not opened neighbours of current tile.
        for (auto i : neighbrs)
          if (!tiles[i].is_open())
            nobrs.emplace_back(i);
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
      }
    }
    return b_state_changed;
  }

  // @brief Flags tiles based on the known 2-1 pattern.
  auto b_pattern_solve_old() {
    bool b_state_changed = false;
    auto &tiles = m_board.m_tiles;

    using pos_type = MineBoard::pos_type;
    std::pair<pos_type, pos_type> res_pos;

    for (size_type i = 0; i < tiles.size(); ++i) {
      if (tiles[i].is_open()) {
        auto neighbrs = m_board.m_tile_neighbours_bnds(i);
        BoardTile::value_type fn2 = 0;
        for (size_type n : neighbrs)
          if (tiles[n].is_flagged())
            ++fn2;
        if (tiles[i].value() - fn2 == BoardTile::TILE_2) {
          auto pos2 = m_board.m_to_pos(i);
          // Set out of bounds initially before tile valued 1 isn't yet found.
          pos_type pos1{static_cast<diff_type>(m_board.width()),
                        static_cast<diff_type>(m_board.height())};
          bool open_tile_infront = false;
          BoardTile::value_type fn1 = 0;
          for (size_type n : neighbrs)
            if (tiles[n].is_flagged())
              ++fn1;
          for (size_type n : neighbrs) {
            if (tiles[n].value() - fn1 == BoardTile::TILE_1) {
              pos1 = m_board.m_to_pos(n);
              if (pos2 - pos1 == pos_type{1, 0}) {
                res_pos.first = pos2 + pos_type{1, -1};
                res_pos.second = pos2 + pos_type{1, 1};
                if (tiles[m_board.m_to_idx(pos2 + pos_type{1, 0})].is_open())
                  open_tile_infront = true;
              } else if (pos2 - pos1 == pos_type{-1, 0}) {
                res_pos.first = pos2 + pos_type{-1, -1};
                res_pos.second = pos2 + pos_type{-1, 1};
                if (tiles[m_board.m_to_idx(pos2 + pos_type{-1, 0})].is_open())
                  open_tile_infront = true;
              } else if (pos2 - pos1 == pos_type{0, 1}) {
                res_pos.first = pos2 + pos_type{-1, 1};
                res_pos.second = pos2 + pos_type{1, 1};
                if (tiles[m_board.m_to_idx(pos2 + pos_type{0, 1})].is_open())
                  open_tile_infront = true;
              } else if (pos2 - pos1 == pos_type{0, -1}) {
                res_pos.first = pos2 + pos_type{-1, -1};
                res_pos.second = pos2 + pos_type{1, -1};
                if (tiles[m_board.m_to_idx(pos2 + pos_type{0, -1})].is_open())
                  open_tile_infront = true;
              }
              if (open_tile_infront) {
                bool b1 = m_board.m_b_inside_bounds(res_pos.first);
                bool b2 = m_board.m_b_inside_bounds(res_pos.second);
                if (b1 && b2) {
                  // This next if statement must be inside the (b1 && b2)
                  // clause, because else if(b1) can't be triggered by situation
                  // where %first and %second positions on the board are both
                  // closed.
                  if (tiles[m_board.m_to_idx(res_pos.first)].is_open() ^
                      tiles[m_board.m_to_idx(res_pos.second)].is_open()) {
                    if (!tiles[m_board.m_to_idx(res_pos.first)].is_open())
                      tiles[m_board.m_to_idx(res_pos.first)].set_flagged();
                    else
                      tiles[m_board.m_to_idx(res_pos.second)].set_flagged();
                    b_state_changed = true;
                  }
                } else if (b1) {
                  tiles[m_board.m_to_idx(res_pos.first)].set_flagged();
                  b_state_changed = true;
                } else if (b2) {
                  tiles[m_board.m_to_idx(res_pos.second)].set_flagged();
                  b_state_changed = true;
                }
                open_tile_infront = false;
              }
            }
          }
        }
      }
    }
    return b_state_changed;
  }

  size_type find_value(BoardTile::value_type tile_value, size_type offset,
                       bool flag_offset = false) {
    auto &tiles = m_board.m_tiles;
    for (size_type i = offset; i < tiles.size(); ++i) {
      if (flag_offset) {
        auto neighbrs = m_board.m_tile_neighbours_bnds(i);
        BoardTile::value_type flagged_neighbours = 0;
        for (auto n : neighbrs)
          if (tiles[n].is_flagged())
            ++flagged_neighbours;
        if (tiles[i].value() - flagged_neighbours == tile_value)
          return i;
      } else if (tiles[i].value() == tile_value)
        return i;
    }
  }

  auto b_pattern_solve() {
    bool b_state_changed = false;
    auto &tiles = m_board.m_tiles;

    using pos_type = MineBoard::pos_type;
    std::pair<pos_type, pos_type> res_pos;

    for (size_type i2 = 0; i2 < tiles.size(); ++i2) {
      if (tiles[i2].is_open()) {
        auto n2 = m_board.m_tile_neighbours_bnds(i2);
        BoardTile::value_type fn2 = 0;
        for (auto n : n2)
          if (tiles[n].is_flagged())
            ++fn2;
        if (tiles[i2].value() - fn2 == BoardTile::TILE_2) {
          auto pos2 = m_board.m_to_pos(i2);
          // Set out of bounds initially before tile valued 1 isn't yet found.
          pos_type pos1{static_cast<diff_type>(m_board.width()),
                        static_cast<diff_type>(m_board.height())};
          bool open_tile_infront = false;
          BoardTile::value_type fn1 = 0;

          // %n2 stands for neighbour of %i2.
          // This loop searches for neighbours of %TILE_2 which have value
          // TILE_1 also taking to account the possible pre-flagged tiles.
          for (size_type in2 = 0; in2 < n2.size() && tiles[n2[in2]].is_open();
               ++in2) {
            auto n1 = m_board.m_tile_neighbours_bnds(n2[in2]);
            fn1 = 0;
            for (auto i : n1)
              if (tiles[i].is_flagged())
                ++fn1;
            if (tiles[n2[in2]].value() - fn1 == BoardTile::TILE_1) {
              open_tile_infront = false;
              pos1 = m_board.m_to_pos(n2[in2]);
              if (pos2 - pos1 == pos_type{1, 0}) {
                res_pos.first = pos2 + pos_type{1, -1};
                res_pos.second = pos2 + pos_type{1, 1};
                if (tiles[m_board.m_to_idx(pos2 + pos_type{1, 0})].is_open())
                  open_tile_infront = true;
              } else if (pos2 - pos1 == pos_type{-1, 0}) {
                res_pos.first = pos2 + pos_type{-1, -1};
                res_pos.second = pos2 + pos_type{-1, 1};
                if (tiles[m_board.m_to_idx(pos2 + pos_type{-1, 0})].is_open())
                  open_tile_infront = true;
              } else if (pos2 - pos1 == pos_type{0, 1}) {
                res_pos.first = pos2 + pos_type{-1, 1};
                res_pos.second = pos2 + pos_type{1, 1};
                if (tiles[m_board.m_to_idx(pos2 + pos_type{0, 1})].is_open())
                  open_tile_infront = true;
              } else if (pos2 - pos1 == pos_type{0, -1}) {
                res_pos.first = pos2 + pos_type{-1, -1};
                res_pos.second = pos2 + pos_type{1, -1};
                if (tiles[m_board.m_to_idx(pos2 + pos_type{0, -1})].is_open())
                  open_tile_infront = true;
              }
              if (open_tile_infront) {
                bool b1 = m_board.m_b_inside_bounds(res_pos.first);
                bool b2 = m_board.m_b_inside_bounds(res_pos.second);
                if (b1 && b2) {
                  // This next if statement must be inside the (b1 && b2)
                  // clause, because else if(b1) can't be triggered by situation
                  // where %first and %second positions on the board are both
                  // closed.
                  if (tiles[m_board.m_to_idx(res_pos.first)].is_open() ^
                      tiles[m_board.m_to_idx(res_pos.second)].is_open()) {
                    if (!tiles[m_board.m_to_idx(res_pos.first)].is_open())
                      tiles[m_board.m_to_idx(res_pos.first)].set_flagged();
                    else
                      tiles[m_board.m_to_idx(res_pos.second)].set_flagged();
                    b_state_changed = true;
                  }
                } else if (b1) {
                  tiles[m_board.m_to_idx(res_pos.first)].set_flagged();
                  b_state_changed = true;
                } else if (b2) {
                  tiles[m_board.m_to_idx(res_pos.second)].set_flagged();
                  b_state_changed = true;
                }
              }
            }
          }
        }
      }
    }
    return b_state_changed;
  }

  // @brief Solves from tiles which share neighbours. Opens those tiles'
  // neighbours which CANNOT be mines.
  /**
   * tile's neighbours complete overlap possible mine positions of another tile.
exluded is not mine.

tile value - flagged neighbours = remaining mines

tile1 neighbours difference tile2 neighbours

tile1 or tile2 contains difference, result -> tileD

tileD value - flagged neighbours == 1
-> open difference

else
-> flag difference
   */
  auto b_common_solve() {
    bool b_state_changed = false;
    auto &tiles = m_board.m_tiles;
    for (size_type i = 0; i < tiles.size(); ++i) {
      if (tiles[i].is_open() && tiles[i].is_number()) {
        auto neighbrs = m_board.m_tile_neighbours_bnds(i);

        for (auto n : neighbrs) {
          if (tiles[n].is_open() && tiles[n].is_number()) {
            auto 
            std::set_difference(neighbrs.begin(), neighbrs.end(),
          }
        }
      }
    }
    return b_state_changed;
  }

  // @brief Tries different combinations and flags based on which results did
  // not fit to the board.
  auto b_suffle_solve() { return false; }

  auto b_solve(size_type start_idx) {
    while (b_overlap_solve() || b_pattern_solve() || b_common_solve())
      ;
    return b_suffle_solve();
  }
}; // namespace rake

} // namespace rake

#endif