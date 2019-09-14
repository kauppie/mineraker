#ifndef MINEBOARDSOLVER_HPP
#define MINEBOARDSOLVER_HPP

#include <algorithm>
#include <limits>
#include <vector>

#include "boardtile.hpp"
#include "mineboard.hpp"
#include "mineraker.hpp"
#include "vectorspace.hpp"

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

  VectorSpace<size_type> m_vecspace;

public:
  MineBoardSolver(MineBoard &board) : m_board(board) {
    m_vecspace.space_size(8);
    m_vecspace.vectors_reserve(8);
  }
  MineBoardSolver(const this_type &other)
      : m_board(other.m_board),
        m_checked_number_tiles(other.m_checked_number_tiles) {}
  MineBoardSolver(this_type &&) = delete;
  ~MineBoardSolver() noexcept {}

  void reset() {
    for (auto checked : m_checked_number_tiles)
      checked = false;
  }

  auto flagged_neighbours_count(size_type idx) {
    size_type count = 0;
    auto neighbrs = m_vecspace.acquire();
    m_board.m_tile_neighbours_bnds(neighbrs.get(), idx);
    for (auto i : neighbrs)
      if (m_board.m_tiles[i].is_flagged())
        ++count;
    return count;
  }

  auto flagged_not_neighbours_count(size_type idx) const {
    size_type count = 0;
    auto neighbrs = m_board.m_tile_neighbours_bnds(idx);
    for (auto i : neighbrs)
      if (!m_board.m_tiles[i].is_flagged())
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

  auto open_not_neighbours_count(size_type idx) const {
    size_type count = 0;
    auto neighbrs = m_board.m_tile_neighbours_bnds(idx);
    for (auto i : neighbrs)
      if (!m_board.m_tiles[i].is_open())
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
    if (m_checked_number_tiles.size() != m_board.tile_count())
      m_checked_number_tiles.resize(m_board.tile_count(), false);

    for (size_type i = 0; i < m_board.tile_count(); ++i) {
      if (tiles[i].is_open() && tiles[i].is_number()) {
        auto flagged_neighbrs_count = flagged_neighbours_count(i);
        // If the amount of flagged tiles equals the value representing mine
        // count on the current tile, open those which are not flagged.
        if (!m_checked_number_tiles[i] &&
            tiles[i].value() == flagged_neighbrs_count) {
          m_board.m_on_next_move(i);
          m_checked_number_tiles[i] = true;
          b_state_changed = true;
        }
      }
    }
    return b_state_changed;
  }

  // Finds first index containing tile with specified value. If %flag_offset is
  // set to true, every tile's flagged neighbours count is substracted from the
  // tile actual value and then compared to given tile value.
  size_type find_value(BoardTile::value_type tile_value, size_type offset = 0,
                       bool flag_offset = false) {
    auto &tiles = m_board.m_tiles;
    if (flag_offset) {
      for (size_type i = offset; i < m_board.tile_count(); ++i) {
        auto neighbrs = m_board.m_tile_neighbours_bnds(i);
        BoardTile::value_type flagged_neighbours = 0;
        for (auto n : neighbrs)
          if (tiles[n].is_flagged())
            ++flagged_neighbours;
        if (tiles[i].value() - flagged_neighbours == tile_value)
          return i;
      }
    } else {
      for (size_type i = offset; i < m_board.tile_count(); ++i) {
        if (tiles[i].value() == tile_value)
          return i;
      }
    }
    return tiles.size();
  }

  // @brief Compares each tile's value to it's unopened neighbour tile count and
  // flag those if they are equal.
  auto b_overlap_solve() {
    bool b_state_changed = false;
    auto &tiles = m_board.m_tiles;
    for (size_type idx = 0; idx < m_board.tile_count(); ++idx) {
      if (tiles[idx].is_open() && tiles[idx].is_number()) {
        // auto neighbrs = m_board.m_tile_neighbours_bnds(idx);
        auto neighbrs = m_vecspace.acquire();
        m_board.m_tile_neighbours_bnds(neighbrs.get(), idx);
        // std::vector<size_type> nobrs;
        auto nobrs = m_vecspace.acquire();
        // Finds not opened neighbours of current tile.
        for (auto &i : neighbrs.get())
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

  // @todo Comment steps in the algorithm.
  auto b_pattern_solve() {
    bool b_state_changed = false;
    auto &tiles = m_board.m_tiles;

    for (size_type i = 0; i < m_board.tile_count(); ++i) {
      if (tiles[i].is_open() && tiles[i].is_number()) {
        auto i_neighbrs = m_vecspace.acquire();
        m_board.m_tile_neighbours_bnds(i_neighbrs.get(), i);
        for (auto n : i_neighbrs) {
          if (tiles[n].is_open() && tiles[n].is_number()) {
            auto neighbrs = m_vecspace.acquire();
            for (auto neigh : i_neighbrs)
              if (!(tiles[neigh].is_open() || tiles[neigh].is_flagged()))
                neighbrs.emplace_back(neigh);

            auto n_neighbrs_open_flagged = m_vecspace.acquire();
            auto n_neighbrs = m_vecspace.acquire();
            m_board.m_tile_neighbours_bnds(n_neighbrs.get(), n);
            for (auto neigh : n_neighbrs)
              if (!(tiles[neigh].is_open() || tiles[neigh].is_flagged()))
                n_neighbrs_open_flagged.emplace_back(neigh);

            // Sort vectors so that %std::set_difference returns correct result.
            std::sort(neighbrs.begin(), neighbrs.end());
            std::sort(n_neighbrs_open_flagged.begin(),
                      n_neighbrs_open_flagged.end());

            auto flag_neighbrs = m_vecspace.acquire();
            std::set_difference(neighbrs.begin(), neighbrs.end(),
                                n_neighbrs_open_flagged.begin(),
                                n_neighbrs_open_flagged.end(),
                                std::back_inserter(flag_neighbrs.get()));
            if (tiles[i].value() - flagged_neighbours_count(i) -
                        tiles[n].value() + flagged_neighbours_count(n) ==
                    1 &&
                flag_neighbrs.size() == 1) {
              tiles[flag_neighbrs.front()].set_flagged();
              b_state_changed = true;
            }
          }
        }
      }
    }
    return b_state_changed;
  }

  // @brief Opens tiles that can't be mines based on tile value pairs.
  // @return Whether something was changed.
  auto b_common_solve() {
    bool b_state_changed = false;
    auto &tiles = m_board.m_tiles;

    // Iterate over tiles.
    for (size_type i = 0; i < m_board.tile_count(); ++i) {
      if (tiles[i].is_open() && tiles[i].is_number()) {
        auto i_neighbrs = m_vecspace.acquire();
        m_board.m_tile_neighbours_bnds(i_neighbrs.get(), i);
        // Iterate over tile's neighbours.
        for (auto n : i_neighbrs) {
          if (tiles[n].is_open() && tiles[n].is_number()) {
            // Construct a vector with tile's neighbours indexes that aren't
            // open nor flagged.
            auto neighbrs_not_open_flagged = m_vecspace.acquire();
            auto neighbrs = m_vecspace.acquire();
            m_board.m_tile_neighbours_bnds(neighbrs.get(), i);
            for (auto neigh : neighbrs)
              if (!(tiles[neigh].is_open() || tiles[neigh].is_flagged()))
                neighbrs_not_open_flagged.emplace_back(neigh);

            // Construct a vector with neighbours's neighbours indexes that
            // aren't open nor flagged.

            // Reuse temporary variable which isn't used anymore.
            neighbrs.get().resize(0);
            m_board.m_tile_neighbours_bnds(neighbrs.get(), n);
            auto n_neighbrs_not_open_flagged = m_vecspace.acquire();
            for (auto neigh : neighbrs)
              if (!(tiles[neigh].is_open() || tiles[neigh].is_flagged()))
                n_neighbrs_not_open_flagged.emplace_back(neigh);

            // %std::set_difference requires sorted data as parameter.
            std::sort(neighbrs_not_open_flagged.begin(),
                      neighbrs_not_open_flagged.end());
            std::sort(n_neighbrs_not_open_flagged.begin(),
                      n_neighbrs_not_open_flagged.end());

            // Extract difference from %neighbrs and %n_neighbrs vectors and
            // insert the result to %diff_neighbrs.
            auto diff_neighbrs = m_vecspace.acquire();
            std::set_difference(neighbrs_not_open_flagged.begin(),
                                neighbrs_not_open_flagged.end(),
                                n_neighbrs_not_open_flagged.begin(),
                                n_neighbrs_not_open_flagged.end(),
                                std::back_inserter(diff_neighbrs.get()));

            // Check whether %n_neighbrs vector is included in the %neighbrs
            // vector and tile's value with flagged neighbours substracted from
            // it equals neighbour's value, also with flagged neighbours
            // substracted from it.
            if (std::includes(neighbrs_not_open_flagged.begin(),
                              neighbrs_not_open_flagged.end(),
                              n_neighbrs_not_open_flagged.begin(),
                              n_neighbrs_not_open_flagged.end()) &&
                tiles[i].value() - flagged_neighbours_count(i) ==
                    tiles[n].value() - flagged_neighbours_count(n)) {
              // Open those tiles that are left over from the possible mine
              // positions.
              for (auto diff : diff_neighbrs) {
                m_board.m_on_next_move(diff);
                b_state_changed = true;
              }
            }
          }
        }
      }
    }
    return b_state_changed;
  }

  // @brief Tries different combinations and flags based on which results did
  // not fit to the board. Brute-force method.
  auto b_suffle_solve() {
    /**
     * Try different combinations spanning flag based on %mines_left and
     * comparing the tiles' values whether they are possible. When all
     * combinations are tried and only 1 solution is found, set those flags.
     */

    auto &tiles = m_board.m_tiles;

    std::vector<size_type> not_opened;
    for (size_type i = 0; i < m_board.tile_count(); ++i)
      if (!(tiles[i].is_open() || tiles[i].is_flagged()))
        not_opened.emplace_back(i);

    std::vector<bool> flag_bits(not_opened.size(), false);
    // rbegin() instead of begin() -> no sorting
    std::fill_n(flag_bits.rbegin(),
                m_board.mine_count() - m_board.flagged_tiles_count(), true);

    if (not_opened.size() > 20)
      return false;

    std::vector<bool> permu_copy;
    size_type ok_count = 0;

    do {
      for (size_type i = 0; i < not_opened.size(); ++i) {
        if (flag_bits[i])
          tiles[not_opened[i]].set_flagged();
        else
          tiles[not_opened[i]].set_unflagged();
      }
      bool ok_combi = true;
      for (size_type i = 0; i < not_opened.size(); ++i) {
        for (auto n : m_board.m_tile_neighbours_bnds(not_opened[i]))
          if (tiles[n].is_open() && tiles[n].is_number() &&
              tiles[n].value() != flagged_neighbours_count(n))
            ok_combi = false;
      }
      if (ok_combi) {
        ++ok_count;
        if (ok_count > 1) {
          for (size_type i = 0; i < not_opened.size(); ++i)
            tiles[not_opened[i]].set_unflagged();
          return false;
        }
        permu_copy = flag_bits;
      }
    } while (std::next_permutation(flag_bits.begin(), flag_bits.end()));

    for (size_type i = 0; i < not_opened.size(); ++i) {
      if (permu_copy[i])
        tiles[not_opened[i]].set_flagged();
      else
        tiles[not_opened[i]].set_unflagged();
    }

    return true;
  }

  auto b_solve() {
    while (b_overlap_solve() || b_common_solve() || b_pattern_solve())
      open_by_flagged();

    return b_suffle_solve();
  }
};

} // namespace rake

#endif