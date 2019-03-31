#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <utility>
#include <vector>

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
  using block_type = unsigned char;
  using tile_type = std::pair<block_type, bool>;
  using vector_type = std::vector<tile_type>;

  // Default container for the board tiles.
  vector_type m_tiles;
  // Random number generator for randomizing tile values.
  std::mt19937_64 rand_engine;
  // Variable for storing the board width.
  std::size_t m_width;

public:
  /*
   * @brief Default constructor with board defining parameters.
   * Through this constructor shape of the board, it's mine count and seed for
   * random number generator are defined.
   */
  SweepBoard(std::size_t width_, std::size_t height_, double mine_fill_percent,
             uint32_t seed = std::mt19937_64::default_seed) {
    set_dimensions(width_, height_);
    init(mine_fill_percent, seed);
  }
  ~SweepBoard() noexcept {}

  // @brief Initializes the board with mine fill percent and a seed for random
  // number generator. Also sets tile values as mines, numbers or emptys.
  void init(double mine_fill_percent, uint32_t seed) {
    rand_engine.seed(seed);
    m_zero_out();
    m_set_mines(mine_fill_percent);
    m_set_numbered_tiles();
  }

  // @brief Sets board dimensions and resizes the container.
  void set_dimensions(std::size_t width_, std::size_t height_) {
    m_width = width_;
    m_tiles.resize(m_width * height_);
  }

  // @brief Sets tile as a mine or not in given position/index.
  // Does bound checking and ignores out-of-bounds conditions.
  void set_mines(std::size_t pos, bool b_mine = true) {
    if (pos < tile_count())
      m_tiles[pos].first = (b_mine ? 9 : 0);
  }

  // @brief Returns the amount mines on the board.
  std::size_t mine_count() const {
    auto count = 0ull;
    for (auto &tile : m_tiles)
      if (tile.first == 9)
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
  void m_zero_out() {
    for (auto &tile : m_tiles)
      tile = std::make_pair(0, false);
  }

  // Calculates mine count from given percentage and distributes them evenly.
  // NOTE: Before calling, board must be empty of mines. Otherwise mine count
  // cannot be guaranteed.
  void m_set_mines(double percent) {
    if (percent <= 0.0)
      return;
    else if (percent >= 1.0) {
      for (auto &tile : m_tiles)
        tile.first = 9;
      return;
    }
    auto mine_count = static_cast<std::size_t>(percent * tile_count());
    for (auto i = 0ull; i < mine_count; ++i) {
      // There's no risk for divide by 0 error.
      auto &block = m_tiles[rand_engine() % tile_count()].first;
      if (block != 9)
        block = 9;
      else
        --i;
    }
  }

  // Sets tiles without mines to have numbers representing how many mines are
  // nearby.
  void m_set_numbered_tiles() {
    for (auto i = m_next_mine(); i < tile_count(); i = m_next_mine(i + 1)) {
      m_promote_tile(i - m_width);
      m_promote_tile(i + m_width);

      if (i % m_width != 0) {
        m_promote_tile(i - m_width - 1);
        m_promote_tile(i - 1);
        m_promote_tile(i + m_width - 1);
      } else if (i % m_width != m_width - 1) {
        m_promote_tile(i - m_width + 1);
        m_promote_tile(i + 1);
        m_promote_tile(i + m_width + 1);
      }
    }
  }

  // Sets single tile at given index to given type.
  void m_set_index_tile(int64_t index, block_type b_type) {
    if (m_b_inside_bounds(index))
      m_tiles[static_cast<std::size_t>(index)].first = b_type;
  }

  // Adds 1 to tile's value unless it's a mine. Empty tile changes to 1.
  // Does bound checking.
  void m_promote_tile(int64_t index) {
    if (m_b_inside_bounds(index)) {
      auto &ref = m_tiles[static_cast<std::size_t>(index)].first;
      ref += (ref < 9 ? 1 : 0);
    }
  }

  // Checks that given index is inside the bounds of board size.
  bool m_b_inside_bounds(int64_t index) const {
    return (index >= 0ll) && (index < static_cast<int64_t>(tile_count()));
  }

  // Returns next tile index with the type of mine starting from optional index.
  // If mine not found until the end of the array, returns ULLONG_MAX
  // (0xffffffffffffffff).
  std::size_t m_next_mine(std::size_t st_idx = 0) const {
    for (; st_idx < tile_count(); ++st_idx) {
      if (m_tiles[st_idx].first == 9)
        return st_idx;
    }
    return 0xffffffffffffffff;
  }
/*
  // @todo Solve by matrix multiplication. Matrix makes a filter
  // which removes the tiles not to add to the list.
  std::vector<std::size_t> m_tile_neighbours(std::size_t idx) const {
    std::vector<std::size_t> rv;
    auto rv = m_tile_neighbour_idxs(idx);
    for (auto i = 0ull; i < rv.size(); ++i) {
      if (!m_b_inside_bounds(rv[i]))
        rv.erase(rv.begin() + i);
    }
    return rv;
  }

  std::vector<std::unique_ptr<tile_type>>
  m_tile_neighbours_ptr(std::size_t idx) const {
    auto vec = m_tile_neighbours(idx);
    std::vector<std::unique_ptr<tile_type>> rv(vec.size());
    for (auto i = 0ull; i < vec.size(); ++i) {
      rv[i] = std::make_unique<tile_type>(m_tiles[vec[i]]);
    }
    return rv;
  }

  std::vector<std::reference_wrapper<tile_type>>
  m_tile_neighbours_ref(std::size_t idx) const {
    auto vec = m_tile_neighbours(idx);
    std::vector<std::reference_wrapper<tile_type>> rv;
    for (auto i : vec)
      rv.emplace_back(m_tiles[i]);
    return rv;
  }

  // @brief Returns a vector containing index's neighbours. Doesn't do bound
  // checking; check m_tile_neighbours for that.
  std::vector<int64_t> m_tile_neighbour_idxs(std::size_t idx) const {
    std::vector<int64_t> rv(static_cast<std::vector<int64_t>::size_type>(8));
    rv[0] = idx - m_width - 1;
    rv[1] = idx - m_width;
    rv[2] = idx - m_width + 1;
    rv[3] = idx - 1;
    rv[4] = idx + 1;
    rv[5] = idx + m_width - 1;
    rv[6] = idx + m_width;
    rv[7] = idx + m_width + 1;
    return rv;
  }

  // @todo Change name to open_tile_neighbours?
  // @bug might cause stack overflow (recursive function call).
  // + all of these stack calls return vector? that's heavy.
  std::vector<std::size_t> m_open_domino_effect(std::size_t idx) {
    if (m_b_inside_bounds(idx)) {
      auto &tile = m_tiles[idx];
      if (tile.second == false) {
        tile.second = true;
        if (tile.first == 0) {
          auto neighb = m_tile_neighbours(idx);
          for (auto n : neighb)
            m_open_domino_effect(n);
          return neighb;
        }
      }
    }
    return std::vector<std::size_t>{};
  }

  // @todo Implement
  // NOTE: This is a worker function for open_domino_effect so that iterative
  // calling is possible. Also possible through lambda inside that function ->
  // investigate if single use.
  bool m_open_empty_neighbours(std::size_t idx, std::vector<std::size_t>& neigbrs) {
    auto vec = m_tile_neighbours_ref(idx);
    if (vec.size() == 0)
      return false;
    for (auto i = 0ull; i < vec.size(); ++i)
      if (vec[i].get().second == false)
        vec[i].get().second = true;
    return true;
  }
*/
  // Returns true if the board is solvable without guessing; false otherwise.
  // TODO: implement
  bool m_b_solvable() const { return false; }
}; // end class SweepBoard