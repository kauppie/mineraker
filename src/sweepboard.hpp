#ifndef SWEEPBOARD_HPP
#define SWEEPBOARD_HPP

#include <random>
#include <stack>
#include <utility>
#include <vector>

#include "board_tile.hpp"

namespace msgn {

/*
 * @brief Class defines a board with tiles which type of empty, number or a
 * mine.
 * @todo
 * - solve member function.
 * - b_solvable member function.
 * - event class inheritance or object
 *   to drive game state. (update) Object called
 *   SweepBoardController has a event handler and
 *   SweepBoard to contain board data.
 */
class SweepBoard {
private:
  using size_type = std::size_t;
  using tile_type = BoardTile;
  using vector_type = std::vector<tile_type>;

  static const unsigned char TILE_NEIGHBOUR_COUNT = 8;

  // Default container for the board tiles.
  vector_type m_tiles;
  // Random number generator for randomizing tile values.
  std::mt19937_64 m_rand_engine;
  // Variable for storing the board width.
  size_type m_width;

  // Adds control for the Control class. Might not be final.
  friend class SweepBoardController;

public:
  // @brief Default constructor without parameters.
  explicit SweepBoard() {}

  // @brief Constructor with board defining parameters.
  // Through this constructor shape of the board, it's mine count and seed for
  // random number generator are defined.
  SweepBoard(
      size_type width, size_type height, double mine_fill,
      std::mt19937_64::result_type seed = std::mt19937_64::default_seed) {
    set_dimensions(width, height);
    init(mine_fill, seed);
  }
  ~SweepBoard() noexcept {}

  // @brief Initializes the board with mine fill percent. Also sets tile values as mines, numbers or emptys.
  void init(double mine_fill) {
    m_zero_out();
    m_set_mines(mine_fill);
    m_set_numbered_tiles();
  }

  // @brief Initializes the board with mine fill percent and a seed for random
  // number generator. Also sets tile values as mines, numbers or emptys.
  void init(double mine_fill, std::mt19937_64::result_type seed) {
    m_set_engine_seed(seed);
    init(mine_fill);
  }

  // @brief Sets board dimensions and resizes the container.
  void set_dimensions(size_type width, size_type height) {
    m_width = width;
    m_tiles.resize(m_width * height);
    m_tiles.shrink_to_fit();
  }

  // @brief Sets tile as a mine or not in given position/index.
  // Does bound checking and ignores out-of-bounds conditions.
  void set_mines(size_type idx, bool b_mine = true) {
    if (m_b_inside_bounds(idx))
      m_tiles[idx].tile_value =
          (b_mine ? BoardTile::TILE_MINE : BoardTile::TILE_EMPTY);
  }

  void open_from_tile(size_type idx) {
    if (m_b_inside_bounds(idx))
      m_flood_open(idx);
  }

  // @brief Returns the amount mines on the board.
  size_type mine_count() const {
    auto count = 0;
    for (auto &tile : m_tiles)
      if (tile.is_mine())
        ++count;
    return count;
  }

  // @brief Returns the width of the board.
  size_type width() const { return m_width; }

  // @brief Returns the height of the board.
  size_type height() const { return tile_count() / width(); }

  // @brief Returns the amount of tiles on the board.
  size_type tile_count() const { return m_tiles.size(); }

  // @brief Returns the amount of area that is filled by mines.
  double fill_percent() const {
    return static_cast<double>(mine_count()) / tile_count();
  }

private:
  // @brief Sets RNG's seed.
  void m_set_engine_seed(std::mt19937_64::result_type seed) {
    m_rand_engine.seed(seed);
  }

  // @brief Sets every tile to an empty one.
  void m_zero_out() {
    for (auto &tile : m_tiles)
      tile.clear();
  }

  // Calculates mine count from given percentage and distributes them evenly.
  // NOTE: Before calling, board must be empty of mines. Otherwise mine count
  // cannot be guaranteed.
  void m_set_mines(double percent) {
    if (percent <= 0.0)
      return;
    else if (percent >= 1.0) {
      for (auto &tile : m_tiles)
        tile.set_mine();
      return;
    }
    auto mine_count = static_cast<size_type>(percent * tile_count());
    for (auto i = 0; i < mine_count; ++i) {
      auto &tile = m_tiles[m_rand_engine() % tile_count()];
      if (tile.is_mine())
        --i;
      tile.set_mine();
    }
  }

  // Sets tiles without mines to have numbers representing how many mines are
  // nearby.
  void m_set_numbered_tiles() {
    for (auto i = m_next_mine(); i < tile_count(); i = m_next_mine(i + 1)) {
      m_promote_tile(i - m_width);
      m_promote_tile(i + m_width);

      // If index isn't against left side wall.
      if (i % m_width != 0) {
        m_promote_tile(i - m_width - 1);
        m_promote_tile(i - 1);
        m_promote_tile(i + m_width - 1);
      }
      // If index isn't against right side wall.
      else if (i % m_width != m_width - 1) {
        m_promote_tile(i - m_width + 1);
        m_promote_tile(i + 1);
        m_promote_tile(i + m_width + 1);
      }
    }
  }

  // Adds 1 to tile's value unless it's a mine. Empty tile changes to 1.
  // Does bound checking.
  void m_promote_tile(size_type idx) {
    if (m_b_inside_bounds(idx)) {
      auto &tile = m_tiles[idx];
      // Adds tile's value.
      tile.tile_value += (tile.tile_value < BoardTile::TILE_8 ? 1 : 0);
    }
  }

  // Checks that given index is inside the bounds of board size.
  bool m_b_inside_bounds(size_type index) const { return index < tile_count(); }

  // Returns next tile index with the type of mine starting from optional index.
  // If mine not found until the end of the array, returns ULLONG_MAX
  // (0xffffffffffffffff).
  size_type m_next_mine(size_type st_idx = 0) const {
    for (; st_idx < tile_count(); ++st_idx) {
      if (m_tiles[st_idx].is_mine())
        return st_idx;
    }
    return 0xffffffffffffffff;
  }

  // @brief Returns the amount of neighbours tile has inside bounds of the
  // board.
  size_type m_neighbour_count(size_type idx) const {
    bool vertical_edge = idx % m_width == 0 || idx % m_width == m_width - 1,
         horizontal_edge = idx < m_width || idx >= height() * (m_width - 1);
    if (vertical_edge && horizontal_edge)
      return 3;
    if (vertical_edge || horizontal_edge)
      return 5;
    return 8;
  }

  // @brief Takes %m_tile_neighbours_idxs as input and does bound checking for
  // it.
  std::vector<size_type> m_tile_neighbours_bnds(size_type idx) const {
    auto rv = m_tile_neighbours_idxs(idx);
    for (auto i = 0; i < rv.size(); ++i)
      if (!m_b_inside_bounds(rv[i]))
        rv.erase(rv.begin() + i);
    return rv;
  }

  // @brief Takes a vector of neighbour indexes and does bound checking for
  // them. Returns said vector as reference.
  std::vector<size_type> &
  m_tile_neighbours_bnds(std::vector<size_type> &neighbr_idxs) const {
    for (auto i = 0; i < neighbr_idxs.size(); ++i)
      if (!m_b_inside_bounds(neighbr_idxs[i]))
        neighbr_idxs.erase(neighbr_idxs.begin() + i);
    return neighbr_idxs;
  }

  // @brief Returns a vector containing index's neighbours. Doesn't do bound
  // checking; check %m_tile_neighbours_bnds for that.
  // @note Returns as a vector so that the output is directly editable in size.
  std::vector<size_type> m_tile_neighbours_idxs(size_type idx) const {
    std::vector<size_type> rv;
    rv.emplace_back(idx - m_width);
    rv.emplace_back(idx + m_width);
    if (idx % m_width != 0) {
      rv.emplace_back(idx - m_width - 1);
      rv.emplace_back(idx - 1);
      rv.emplace_back(idx + m_width - 1);
    } else if (idx % m_width != m_width - 1) {
      rv.emplace_back(idx - m_width + 1);
      rv.emplace_back(idx + 1);
      rv.emplace_back(idx + m_width + 1);
    }
    return rv;
  }

  void m_flood_open(size_type idx) {
    m_tiles[idx].set_open();
    m_open_neighbours(m_empty_tiles_empty_area(idx));
  }

  std::vector<size_type> m_empty_tiles_empty_area(size_type idx) const {
    if (!m_tiles[idx].is_empty())
      return std::vector<size_type>{};

    std::vector<size_type> rv;
    std::stack<size_type> st_neigh;
    std::vector<bool> checked_tiles(tile_count(), false);

    st_neigh.emplace(idx);
    while (!st_neigh.empty()) {
      idx = st_neigh.top();
      st_neigh.pop();
      checked_tiles[idx] = true;
      rv.emplace_back(idx);
      for (auto n : m_tile_neighbours_idxs(idx))
        if (m_b_inside_bounds(n) && m_tiles[n].is_empty() && !checked_tiles[n])
          st_neigh.emplace(n);
    }
    return rv;
  }

  bool m_open_neighbours(size_type idx) {
    auto neighbrs = m_tile_neighbours_bnds(idx);
    if (neighbrs.size() == 0)
      return false;
    for (auto i : neighbrs)
      m_tiles[i].set_open();
    return true;
  }

  bool m_open_neighbours(const std::vector<size_type> &neighbrs) {
    if (neighbrs.size() == 0)
      return false;
    for (auto i : neighbrs)
      m_tiles[i].set_open();
    return true;
  }

  // Returns true if the board is solvable without guessing; false otherwise.
  // TODO: implement
  bool m_b_solvable() const { return false; }
}; // class SweepBoard

} // namespace msgn

#endif