#ifndef SWEEPBOARD_HPP
#define SWEEPBOARD_HPP

#include <functional>
#include <random>
#include <stack>
#include <utility>
#include <vector>

#include "boardtile.hpp"

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
  using this_type = SweepBoard;

  static const unsigned char TILE_NEIGHBOUR_COUNT = 8;

  // Default container for the board tiles.
  vector_type m_tiles;
  // Variable for storing the board width.
  size_type m_width;
  // Seed for mined tile position randomization.
  std::mt19937_64::result_type m_seed;
  size_type m_mine_count;

  // Adds control for the Control class. Might not be final.
  friend class SweepBoardController;
  // Allows formatter to access private methods and variables.
  friend class SweepBoardFormat;

public:
  // @brief Default constructor without parameters.
  explicit SweepBoard() {}

  // @brief Constructor with board defining parameters.
  // Through this constructor shape of the board, it's mine count and seed for
  // random number generator are defined.
  explicit SweepBoard(
      size_type width, size_type height, double mine_fill,
      std::mt19937_64::result_type seed = std::mt19937_64::default_seed)
      : m_seed(seed) {
    set_dimensions(width, height);
    init(mine_fill);
  }
  // @brief Constructor with board defining parameters.
  // Through this constructor shape of the board, it's mine count and seed for
  // random number generator are defined.
  explicit SweepBoard(
      size_type width, size_type height, size_type mine_count,
      std::mt19937_64::result_type seed = std::mt19937_64::default_seed)
      : m_seed(seed) {
    set_dimensions(width, height);
    init(mine_count);
  }
  ~SweepBoard() noexcept {}

  auto seed(std::mt19937_64::result_type seed) {
    auto old = m_seed;
    m_seed = seed;
    return old;
  }

  constexpr std::mt19937_64::result_type seed() const noexcept {
    return m_seed;
  }

  // @brief Initializes the board with mine fill percent. Also sets tile values
  // as mines, numbers or emptys.
  void init(double mine_fill) {
    init(static_cast<size_type>(mine_fill * tile_count()));
  }

  // @brief Initializes the board with mine fill percent. Also sets tile values
  // as mines, numbers or emptys.
  void init(size_type mine_count) {
    m_clear();
    m_set_mines(m_seed, mine_count);
    m_set_numbered_tiles();
  }

  // @brief Initializes the board with mine fill percent. Also sets tile values
  // as mines, numbers or emptys.
  void init() {
    m_clear();
    m_set_mines(m_seed, m_mine_count);
    m_set_numbered_tiles();
  }

  // @brief Sets board dimensions and resizes the container.
  void set_dimensions(size_type width, size_type height) {
    m_width = width;
    m_tiles.resize(width * height);
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

  // @brief Returns the width of the board.
  constexpr size_type width() const noexcept { return m_width; }

  // @brief Returns the height of the board.
  size_type height() const noexcept { return tile_count() / width(); }

  // @brief Returns the amount mines on the board.
  constexpr size_type mine_count() const noexcept { return m_mine_count; }

  // @brief Returns the amount of tiles on the board.
  size_type tile_count() const noexcept { return m_tiles.size(); }

  // @brief Returns the amount of area that is filled by mines.
  double fill_percent() const noexcept {
    return static_cast<double>(mine_count()) / tile_count();
  }

private:
  // @brief Sets every tile to an empty one.
  void m_clear() {
    for (auto &tile : m_tiles)
      tile.clear();
  }

  // @brief Calculates mine count from given percentage and distributes them
  // evenly.
  // @note Before calling, board must be empty of mines. Otherwise mine count
  // cannot be guaranteed.
  void m_set_mines(std::mt19937_64::result_type seed, size_type mine_count) {
    m_mine_count = std::min(mine_count, tile_count());
    if (mine_count >= tile_count()) {
      for (auto &tile : m_tiles)
        tile.set_mine();
      return;
    }
    // Random number generator for random mine positions.
    std::mt19937_64 rng(seed);
    for (auto i = 0ull; i < mine_count; ++i) {
      auto &tile = m_tiles[rng() % tile_count()];
      // Reduce %i because the amount of mines won't change.
      if (tile.is_mine())
        --i;
      tile.set_mine();
    }
  }

  // @brief Sets tiles without mines to have numbers representing how many mines
  // are nearby.
  void m_set_numbered_tiles() {
    for (auto i = m_next_mine(); i < tile_count(); i = m_next_mine(i + 1)) {
      m_promote_tile(i - m_width);
      m_promote_tile(i + m_width);
      // If (index isn't against the right side wall). These indexes wrap around
      // the board to the otherside if %idx is next to the left side wall.
      if (i % m_width != 0) {
        m_promote_tile(i - m_width - 1);
        m_promote_tile(i - 1);
        m_promote_tile(i + m_width - 1);
      }
      // If (index isn't against the right side wall). These indexes wrap around
      // the board to the otherside if %idx is next to the right side wall.
      if (i % m_width != m_width - 1) {
        m_promote_tile(i - m_width + 1);
        m_promote_tile(i + 1);
        m_promote_tile(i + m_width + 1);
      }
    }
  }

  // @brief Adds 1 to tile's value unless it's a mine. Empty tile changes to 1.
  // Does bound checking.
  void m_promote_tile(size_type idx) {
    if (m_b_inside_bounds(idx)) {
      auto &tile = m_tiles[idx];
      // Adds tile's value.
      tile.tile_value += (tile.tile_value < BoardTile::TILE_8 ? 1 : 0);
    }
  }

  // @brief Checks that given index is inside the bounds of board size.
  bool m_b_inside_bounds(size_type index) const { return index < tile_count(); }

  // @brief Returns next tile index with the type of mine starting from optional
  // index. If mine not found until the end of the array, returns ULLONG_MAX
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
    // If is against both vertically and horizontally going walls.
    if (vertical_edge && horizontal_edge)
      return 3;
    // If is against either vertically or horizontally going wall.
    if (vertical_edge || horizontal_edge)
      return 5;
    // If isn't against any walls.
    return 8;
  }

  // @brief Takes %m_tile_neighbours_idxs as input and does bound checking for
  // it.
  std::vector<size_type> m_tile_neighbours_bnds(size_type idx) const {
    auto rv = m_tile_neighbours_idxs(idx);
    for (auto i = 0ull; i < rv.size(); ++i)
      if (!m_b_inside_bounds(rv[i]))
        rv.erase(rv.begin() + i);
    return rv;
  }

  // @brief Takes a vector of neighbour indexes and does bound checking for
  // the contained indexes. Returns said vector as reference.
  std::vector<size_type> &
  m_tile_neighbours_bnds(std::vector<size_type> &neighbr_idxs) const {
    for (auto i = 0ull; i < neighbr_idxs.size(); ++i)
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
    // If (index isn't against the right side wall). These indexes wrap around
    // the board to the otherside if %idx is next to the left side wall.
    if (idx % m_width != 0) {
      rv.emplace_back(idx - m_width - 1);
      rv.emplace_back(idx - 1);
      rv.emplace_back(idx + m_width - 1);
    }
    // If (index isn't against the right side wall). These indexes wrap around
    // the board to the otherside if %idx is next to the right side wall.
    if (idx % m_width != m_width - 1) {
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

  // @brief Returns vector of empty tiles that are neighbouring each other
  // starting from given index.
  std::vector<size_type> m_empty_tiles_empty_area(size_type idx) const {
    if (!m_tiles[idx].is_empty())
      return std::vector<size_type>{};

    // Vector to be returned.
    std::vector<size_type> rv;
    // Stack for storing neighbouring tile's which are to be checked.
    std::stack<size_type> st_neigh;
    // Stores which tiles are already run by the loop.
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

  // @brief Opens neighbours of a tile at the given index.
  bool m_open_neighbours(size_type idx) {
    return m_open_neighbours(m_tile_neighbours_bnds(idx));
  }

  // @brief Takes vector of indexes and opens tiles represented by them.
  bool m_open_neighbours(const std::vector<size_type> &neighbrs) {
    if (neighbrs.size() == 0)
      return false;
    for (auto i : neighbrs)
      m_tiles[i].set_open();
    return true;
  }

  // @brief Returns true if the board is solvable without guessing. False
  // otherwise.
  // @todo Implement.
  bool m_b_solvable() const { return false; }
}; // class SweepBoard

} // namespace msgn

#endif