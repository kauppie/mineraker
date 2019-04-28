#ifndef MINEBOARD_HPP
#define MINEBOARD_HPP

#include <functional>
#include <limits>
#include <random>
#include <stack>
#include <utility>
#include <vector>

#include "boardtile.hpp"

namespace rake {

/*
 * @brief Class defines a board with tiles which type of empty, number or a
 * mine.
 * @todo
 * - solve member function.
 * - b_solvable member function.
 * - event class inheritance or object
 *   to drive game state. (update) Object called
 *   MineBoardController has a event handler and
 *   MineBoard to contain board data.
 */
class MineBoard {
public:
  using size_type = std::size_t;
  using tile_type = BoardTile;
  using vector_type = std::vector<tile_type>;
  using this_type = MineBoard;

  static const unsigned char TILE_NEIGHBOUR_COUNT = 8;

private:
  // Default container for the board tiles.
  vector_type m_tiles;
  // Variable for storing the board width.
  size_type m_width;
  // Seed for mined tile position randomization.
  std::mt19937_64::result_type m_seed;
  // Stores the amount of mines on the board.
  size_type m_mine_count;

  // Adds control for the Control class. Might not be final.
  friend class MineBoardController;
  // Allows formatter to access private methods and variables.
  friend class MineBoardFormat;
  // Allows solver to access private information needed for solving the board.
  friend class MineBoardSolver;

public:
  // @brief Default constructor without parameters.
  MineBoard() {}

  // @brief Constructor with board defining parameters.
  // Through this constructor shape of the board, it's mine count and seed for
  // random number generator are defined.
  MineBoard(size_type width, size_type height, size_type mine_count,
            std::mt19937_64::result_type seed = std::mt19937_64::default_seed)
      : m_width(0), m_seed(seed), m_mine_count(0) {
    set_dimensions(width, height);
    init(mine_count);
  }
  MineBoard(const this_type &other)
      : m_tiles(other.m_tiles), m_width(other.m_width), m_seed(other.m_seed),
        m_mine_count(other.m_mine_count) {}
  MineBoard(this_type &&other) noexcept
      : m_tiles(std::move(other.m_tiles)), m_width(std::move(other.m_width)),
        m_seed(std::move(other.m_seed)),
        m_mine_count(std::move(other.m_mine_count)) {}
  ~MineBoard() noexcept {}

  this_type &operator=(const this_type &other) {
    m_tiles = other.m_tiles;
    m_width = other.m_width;
    m_seed = other.m_seed;
    m_mine_count = other.m_mine_count;

    return *this;
  }

  this_type &&operator=(this_type &&other) noexcept {
    m_tiles = std::move(other.m_tiles);
    m_width = std::move(other.m_width);
    m_seed = std::move(other.m_seed);
    m_mine_count = std::move(other.m_mine_count);

    return std::move(*this);
  }

  auto seed(std::mt19937_64::result_type seed) {
    auto old = m_seed;
    m_seed = seed;
    return old;
  }

  constexpr auto seed() const noexcept { return m_seed; }

  // @brief Initializes the board with mine fill percent. Also sets tile values
  // as mines, numbers or emptys.
  void init(size_type mine_count, size_type no_mine_idx) {
    m_clear();
    m_set_mines(mine_count, no_mine_idx);
    m_set_numbered_tiles();
  }

  // @brief Initializes the board with mine fill percent. Also sets tile values
  // as mines, numbers or emptys.
  void init(size_type no_mine_idx) {
    m_clear();
    m_set_mines(m_mine_count, no_mine_idx);
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
    if (m_b_inside_bounds(idx) && !m_tiles[idx].b_flagged &&
        !m_tiles[idx].b_open)
      m_flood_open(idx);
  }

  // @brief Returns the width of the board.
  constexpr auto width() const noexcept { return m_width; }

  // @brief Returns the height of the board.
  size_type height() const noexcept { return tile_count() / width(); }

  // @brief Returns the amount mines on the board.
  constexpr auto mine_count() const noexcept { return m_mine_count; }

  // @brief Returns the amount of tiles on the board.
  size_type tile_count() const noexcept { return m_tiles.size(); }

  // @brief Returns the amount of area that is filled by mines.
  double fill_percent() const noexcept {
    return static_cast<double>(mine_count()) / tile_count();
  }

private:
  size_type to_idx(size_type x, size_type y) const noexcept {
    return y * m_width + x;
  }

  // @brief Sets every tile to an empty one.
  void m_clear() {
    for (auto &tile : m_tiles)
      tile.clear();
  }

  // @brief Calculates mine count from given count and distributes them
  // evenly. %no_mine_idx points to tile which won't filled by a mine.
  // @note Before calling, board must be empty of mines. Otherwise mine count
  // cannot be guaranteed.
  void m_set_mines(size_type mine_count, size_type no_mine_idx) {
    m_mine_count = std::min(mine_count, tile_count());
    if (mine_count >= tile_count()) {
      for (auto &tile : m_tiles)
        tile.set_mine();
      m_tiles[no_mine_idx].tile_value = BoardTile::TILE_8;
      return;
    }
    // Random number generator for random mine positions.
    std::mt19937_64 rng(m_seed);
    for (auto i = 0ull; i < mine_count; ++i) {
      auto idx = rng() % tile_count();
      if (idx != no_mine_idx || !m_tiles[idx].is_mine())
        m_tiles[idx].set_mine();
      // Reduce %i because the amount of mines won't change.
      else
        --i;
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
    if (m_b_inside_bounds(idx))
      m_tiles[idx].promote();
  }

  // @brief Checks that given index is inside the bounds of board size.
  bool m_b_inside_bounds(size_type index) const { return index < tile_count(); }

  // @brief Returns next tile index with the type of mine starting from optional
  // index. If mine not found until the end of the array, returns the maximum
  // value of size_type.
  size_type m_next_mine(size_type st_idx = 0) const {
    for (; st_idx < tile_count(); ++st_idx) {
      if (m_tiles[st_idx].is_mine())
        return st_idx;
    }
    return std::numeric_limits<size_type>::max();
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

  // @brief Returns bounds checked neighbours.
  std::vector<size_type> m_tile_neighbours_bnds(size_type idx) const {
    std::vector<size_type> rv;
    const bool up_edge = idx >= m_width && idx < tile_count(),
               bottom_edge = idx < tile_count() - m_width;
    if (up_edge)
      rv.emplace_back(idx - m_width);
    if (bottom_edge)
      rv.emplace_back(idx + m_width);
    // If (index isn't against the right side wall). These indexes wrap around
    // the board to the otherside if %idx is next to the left side wall.
    if (idx % m_width != 0) {
      if (up_edge)
        rv.emplace_back(idx - m_width - 1);
      rv.emplace_back(idx - 1);
      if (bottom_edge)
        rv.emplace_back(idx + m_width - 1);
    }
    // If (index isn't against the right side wall). These indexes wrap around
    // the board to the otherside if %idx is next to the right side wall.
    if (idx % m_width != m_width - 1) {
      if (up_edge)
        rv.emplace_back(idx - m_width + 1);
      rv.emplace_back(idx + 1);
      if (bottom_edge)
        rv.emplace_back(idx + m_width + 1);
    }
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
      for (auto n : m_tile_neighbours_bnds(idx))
        if (m_tiles[n].is_empty() && !checked_tiles[n])
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
}; // class MineBoard

} // namespace rake

#endif