#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <stack>
#include <utility>
#include <vector>

#include "board_tile.h"

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
  using tile_type = BoardTile;
  using vector_type = std::vector<tile_type>;

  static const unsigned char TILE_NEIGHBOUR_COUNT = 8;

  // Default container for the board tiles.
  vector_type m_tiles;
  // Random number generator for randomizing tile values.
  std::mt19937_64 m_rand_engine;
  // Variable for storing the board width.
  std::size_t m_width;

  // Adds control for the Control class. Might not be final.
  friend class SweepBoardController;

public:
  /*
   * @brief Default constructor with board defining parameters.
   * Through this constructor shape of the board, it's mine count and seed for
   * random number generator are defined.
   */
  SweepBoard(
      std::size_t width, std::size_t height, double mine_fill,
      std::mt19937_64::result_type seed = std::mt19937_64::default_seed) {
    set_dimensions(width, height);
    init(mine_fill, seed);
  }
  SweepBoard() = delete;
  ~SweepBoard() noexcept {}

  // @brief Initializes the board with mine fill percent and a seed for random
  // number generator. Also sets tile values as mines, numbers or emptys.
  void init(double mine_fill, std::mt19937_64::result_type seed) {
    m_rand_engine.seed(seed);
    m_zero_out();
    m_set_mines(mine_fill);
    m_set_numbered_tiles();
  }

  // @brief Sets board dimensions and resizes the container.
  void set_dimensions(std::size_t width_, std::size_t height_) {
    m_width = width_;
    m_tiles.resize(m_width * height_);
    m_tiles.shrink_to_fit();
  }

  // @brief Sets tile as a mine or not in given position/index.
  // Does bound checking and ignores out-of-bounds conditions.
  void set_mines(std::size_t pos, bool b_mine = true) {
    if (pos < tile_count())
      m_tiles[pos].tile_value =
          (b_mine ? BoardTile::TILE_MINE : BoardTile::TILE_EMPTY);
  }

  // @brief Returns the amount mines on the board.
  std::size_t mine_count() const {
    auto count = 0ull;
    for (auto &tile : m_tiles)
      if (tile.is_mine())
        ++count;
    return count;
  }

  // @brief Returns the width of the board.
  std::size_t width() const { return m_width; }

  // @brief Returns the height of the board.
  std::size_t height() const { return tile_count() / width(); }

  // @brief Returns the amount of tiles on the board.
  std::size_t tile_count() const { return m_tiles.size(); }

  // @brief Returns the amount of area that is filled by mines.
  double fill_percent() const {
    return static_cast<double>(mine_count()) / tile_count();
  }

private:
public:
  // @brief Sets every tile to an empty one.
  void m_zero_out() {
    for (auto &tile : m_tiles)
      tile.reset();
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
    auto mine_count = static_cast<std::size_t>(percent * tile_count());
    for (auto i = 0ull; i < mine_count; ++i) {
      auto &tile = m_tiles[m_rand_engine() % tile_count()];
      if (tile.is_mine())
        --i;
      tile.set_mine();
    }
  }

  // Sets tiles without mines to have numbers representing how many mines are
  // nearby.
  void m_set_numbered_tiles() {
    for (auto i = m_next_mine(); i < tile_count(); i = m_next_mine(i + 1ull)) {
      m_promote_tile(i - m_width);
      m_promote_tile(i + m_width);

      // If index isn't against left side wall.
      if (i % m_width != 0) {
        m_promote_tile(i - m_width - 1ull);
        m_promote_tile(i - 1ull);
        m_promote_tile(i + m_width - 1ull);
      }
      // If index isn't against right side wall.
      else if (i % m_width != m_width - 1ull) {
        m_promote_tile(i - m_width + 1ull);
        m_promote_tile(i + 1ull);
        m_promote_tile(i + m_width + 1ull);
      }
    }
  }

  // Adds 1 to tile's value unless it's a mine. Empty tile changes to 1.
  // Does bound checking.
  void m_promote_tile(std::size_t idx) {
    if (m_b_inside_bounds(idx)) {
      auto &tile = m_tiles[idx];
      // Adds tile's value.
      tile.tile_value += (tile.tile_value < BoardTile::TILE_8 ? 1 : 0);
    }
  }

  // Checks that given index is inside the bounds of board size.
  bool m_b_inside_bounds(std::size_t index) const {
    return (index >= 0ll) && (index < tile_count());
  }

  // Returns next tile index with the type of mine starting from optional index.
  // If mine not found until the end of the array, returns ULLONG_MAX
  // (0xffffffffffffffff).
  std::size_t m_next_mine(std::size_t st_idx = 0ull) const {
    for (; st_idx < tile_count(); ++st_idx) {
      if (m_tiles[st_idx].is_mine())
        return st_idx;
    }
    return 0xffffffffffffffff;
  }

  // @brief Returns the amount of neighbours tile has inside bounds of the
  // board.
  std::size_t m_neighbour_count(std::size_t idx) const {
    bool vertical_edge =
             idx % m_width == 0ull || idx % m_width == m_width - 1ull,
         horizontal_edge = idx < m_width || idx >= height() * (m_width - 1ull);
    if (vertical_edge && horizontal_edge)
      return 3ull;
    if (vertical_edge || horizontal_edge)
      return 5ull;
    return 8ull;
  }

  // @brief Takes %m_tile_neighbours_idxs as input and does bound checking for
  // it.
  std::vector<std::size_t> m_tile_neighbours_bnds(std::size_t idx) const {
    auto rv = m_tile_neighbours_idxs(idx);
    for (auto i = 0ull; i < rv.size(); ++i)
      if (!m_b_inside_bounds(rv[i]))
        rv.erase(rv.begin() + i);
    return rv;
  }

  // @brief Takes a vector of neighbour indexes and does bound checking for
  // them. Returns said vector as reference.
  std::vector<std::size_t> &
  m_tile_neighbours_bnds(std::vector<std::size_t> &neighbr_idxs) {
    for (auto i = 0ull; i < neighbr_idxs.size(); ++i)
      if (!m_b_inside_bounds(neighbr_idxs[i]))
        neighbr_idxs.erase(neighbr_idxs.begin() + i);
    return neighbr_idxs;
  }

  // @brief Returns pointers to tile's neighbours. Each pointer is valid.
  std::vector<std::unique_ptr<tile_type>>
  m_tile_neighbours_bnds_ptr(std::size_t idx) const {
    auto vec = m_tile_neighbours_bnds(idx);
    std::vector<std::unique_ptr<tile_type>> rv(vec.size());
    for (auto i = 0ull; i < vec.size(); ++i)
      rv[i] = std::make_unique<tile_type>(m_tiles[vec[i]]);
    return rv;
  }

  // @brief Returns a vector containing index's neighbours. Doesn't do bound
  // checking; check %m_tile_neighbours_bnds for that.
  // @note Returns as a vector so that the output is directly editable in size.
  std::vector<std::size_t> m_tile_neighbours_idxs(std::size_t idx) const {
    std::vector<std::size_t> rv;
    rv.emplace_back(idx - m_width);
    rv.emplace_back(idx + m_width);
    if (idx % m_width != 0ull) {
      rv.emplace_back(idx - m_width - 1ull);
      rv.emplace_back(idx - 1ull);
      rv.emplace_back(idx + m_width - 1ull);
    } else if (idx % m_width != m_width - 1ull) {
      rv.emplace_back(idx - m_width + 1ull);
      rv.emplace_back(idx + 1ull);
      rv.emplace_back(idx + m_width + 1ull);
    }
    return rv;
  }

  // @brief Opens tiles starting from given index. If tile is valued mine or a
  // number, it is opened but nothing else. If tile is empty, its all
  // neighbours are opened as well and same operation starts all over again from
  // neighbouring empty tiles.
  // @note Is being replaced by m_empty_tiles_empty_area() and
  // m_flood_open_new() for their 5% better performance and flexibility.
  void m_flood_open(std::size_t idx) {
    if (m_b_inside_bounds(idx)) {
      m_tiles[idx].set_open();
      if (m_tiles[idx].is_empty()) {
        // Vector for keeping note of which tiles have been already checked.
        std::vector<bool> checked_tiles(tile_count(), false);
        // Stack for all to-be-opened tiles.
        std::stack<std::size_t> st_open;
        st_open.emplace(idx);
        while (!st_open.empty()) {
          idx = st_open.top();
          st_open.pop();
          auto neighbrs = m_tile_neighbours_bnds(idx);
          m_open_neighbours(neighbrs);
          checked_tiles[idx] = true;
          for (auto n : neighbrs)
            if (m_tiles[n].is_empty() && !checked_tiles[n])
              st_open.emplace(n);
        }
      }
    }
  }

  void m_flood_open_new(std::size_t idx) {
    if (!m_b_inside_bounds(idx))
      return;
    m_tiles[idx].set_open();
    m_open_neighbours(m_empty_tiles_empty_area(idx));
  }

  std::vector<std::size_t> m_empty_tiles_empty_area(std::size_t idx) {
    if (!m_b_inside_bounds(idx) || !m_tiles[idx].is_empty())
      return std::vector<std::size_t>();

    std::vector<std::size_t> rv;
    std::stack<std::size_t> st_neigh;
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

  // NOTE: This is a worker function for %m_open_domino_effect so that iterative
  // calling is possible. Also possible through lambda inside that function ->
  // investigate if single use. Neighbours are bound checked but %idx is not.
  bool m_open_area(std::size_t idx) {
    bool changes = false;
    if (!m_tiles[idx].b_open) {
      m_tiles[idx].set_open();
      changes = true;
      if (m_tiles[idx].is_empty()) {
        auto vec = m_tile_neighbours_bnds_ptr(idx);
        for (auto &tile : vec)
          tile.get()->set_open();
      }
    }
    return changes;
  }

  bool m_open_neighbours(std::size_t idx) {
    auto neighbrs = m_tile_neighbours_bnds(idx);
    if (neighbrs.size() == 0ull)
      return false;
    for (auto i : neighbrs)
      m_tiles[i].set_open();
    return true;
  }

  bool m_open_neighbours(
      /* const */ std::vector<std::unique_ptr<tile_type>> &neighbrs) {
    if (neighbrs.size() == 0ull)
      return false;
    for (auto &tile : neighbrs)
      tile.get()->set_open();
    return true;
  }

  bool m_open_neighbours(const std::vector<std::size_t> &neighbrs) {
    if (neighbrs.size() == 0ull)
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