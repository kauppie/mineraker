#ifndef MINEBOARD_HPP
#define MINEBOARD_HPP

#include <exception>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <random>
#include <stack>
#include <utility>
#include <vector>

#include "boardtile.hpp"
#include "mineraker.hpp"

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
  using tile_type = BoardTile;
  using this_type = MineBoard;

  struct pos_type_t {
    diff_type x, y;

    // @brief Addition.
    constexpr pos_type_t operator+(pos_type_t other) const noexcept {
      return {x + other.x, y + other.y};
    }

    // @brief Substraction.
    constexpr pos_type_t operator-(pos_type_t other) const noexcept {
      return {x - other.x, y - other.y};
    }

    // @brief Unary plus.
    constexpr pos_type_t operator+() const noexcept { return *this; }

    // @brief Unary minus.
    constexpr pos_type_t operator-() const noexcept { return {-x, -y}; }

    // @brief Addition assignment operator implementation.
    constexpr pos_type_t operator+=(pos_type_t other) noexcept {
      *this = {x + other.x, y + other.y};
      return *this;
    }

    // @brief Substraction assignment operator implementation.
    constexpr pos_type_t operator-=(pos_type_t other) noexcept {
      *this = {x - other.x, y - other.y};
      return *this;
    }

    // @brief Compares this and other position types. First y is compared and if
    // not equal, return. Otherwise return comparison between x.
    // @note Enable constexpr when lambdas can return constexpr.
    static int compare(pos_type_t lhs, pos_type_t rhs) noexcept {
      auto comp = [](diff_type v1, diff_type v2) -> int {
        return (v1 < v2 ? -1 : (v2 < v1 ? 1 : 0));
      };
      auto ycomp = comp(lhs.y, rhs.y);
      return (ycomp != 0 ? ycomp : comp(lhs.x, rhs.x));
    }
  };

  enum State { UNINITIALIZED = -1, READY = 0, ERROR = 1, GAMEOVER = 10 };

  static const unsigned char TILE_NEIGHBOUR_COUNT = 8;

private:
  // Default container for the board tiles.
  std::vector<tile_type> m_tiles;
  // Variable for storing the board width.
  size_type m_width;
  // Variable for storing the board height.
  size_type m_height;
  // Seed for mined tile position randomization.
  std::mt19937_64::result_type m_seed;
  // Stores the amount of mines on the board.
  size_type m_mine_count;
  // Represented current state of the board.
  State m_state;

  // Adds control for the Control class. Might not be final.
  friend class MineBoardController;
  // Allows formatter to access private methods and variables.
  friend class MineBoardFormat;
  // Allows solver to access private information needed for solving the board.
  friend class MineBoardSolver;

public:
  // @brief Default constructor without parameters.
  MineBoard()
      : m_width(1), m_height(1), m_seed(std::mt19937_64::default_seed),
        m_mine_count(0), m_state(UNINITIALIZED) {}

  // @brief Constructor with board defining parameters.
  // Through this constructor shape of the board, it's mine count and seed for
  // random number generator are defined.
  MineBoard(size_type width, size_type height, size_type mine_count,
            std::mt19937_64::result_type seed = std::mt19937_64::default_seed)
      : m_width(0), m_seed(seed), m_mine_count(mine_count),
        m_state(UNINITIALIZED) {
    set_dimensions(width, height);
  }
  MineBoard(const this_type &other)
      : m_tiles(other.m_tiles), m_width(other.m_width),
        m_height(other.m_height), m_seed(other.m_seed),
        m_mine_count(other.m_mine_count) {}
  MineBoard(this_type &&other) noexcept
      : m_tiles(std::move(other.m_tiles)), m_width(std::move(other.m_width)),
        m_height(std::move(other.m_height)), m_seed(std::move(other.m_seed)),
        m_mine_count(std::move(other.m_mine_count)) {}
  ~MineBoard() noexcept {}

  this_type &operator=(const this_type &other) {
    m_tiles = other.m_tiles;
    m_width = other.m_width;
    m_height = other.m_height;
    m_seed = other.m_seed;
    m_mine_count = other.m_mine_count;

    return *this;
  }

  this_type &&operator=(this_type &&other) noexcept {
    m_tiles = std::move(other.m_tiles);
    m_width = std::move(other.m_width);
    m_height = std::move(other.m_height);
    m_seed = std::move(other.m_seed);
    m_mine_count = std::move(other.m_mine_count);

    return std::move(*this);
  }

  auto seed(std::mt19937_64::result_type seed) noexcept {
    auto old = m_seed;
    m_seed = seed;
    return old;
  }

  constexpr auto seed() const noexcept { return m_seed; }

  constexpr auto state() const noexcept { return m_state; }

  // @brief Initializes the board with mine fill percent. Also sets tile values
  // as mines, numbers or emptys.
  void init(size_type no_mine_idx, size_type mine_count) {
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
    width = std::max(width, static_cast<size_type>(1));
    height = std::max(height, static_cast<size_type>(1));
    m_width = width;
    m_height = height;
    try {
      m_tiles.reserve(m_width * m_height);
      m_tiles.resize(m_width * m_height);
    } catch (std::exception &e) {
      std::cout << "Error: Couldn't reserve memory for mineboard.\n";
      throw e;
    }
  }

  void open_from_tile(size_type idx) {
    if (m_state == UNINITIALIZED)
      init(idx);
    if (m_b_inside_bounds(idx) && !m_tiles[idx].is_flagged() &&
        !m_tiles[idx].is_open())
      m_flood_open(idx);
  }

  // @brief Returns the width of the board.
  constexpr auto width() const noexcept { return m_width; }

  // @brief Returns the height of the board.
  constexpr auto height() const noexcept { return m_height; }

  // @brief Returns the amount mines on the board.
  constexpr auto mine_count() const noexcept { return m_mine_count; }

  // @brief Returns the amount of tiles on the board.
  constexpr auto tile_count() const noexcept { return m_width * m_height; }

  // @brief Returns the amount of area that is filled by mines.
  constexpr double fill_percent() const noexcept {
    return static_cast<double>(mine_count()) / tile_count();
  }

  auto open_tiles_count() const noexcept {
    size_type open_tiles = 0;
    for (auto &tile : m_tiles)
      if (tile.is_open())
        ++open_tiles;
    return open_tiles;
  }

  // @brief Returns the amount of neighbours tiles have combined.
  constexpr auto neighbours_of_tiles_count() const noexcept {
    return m_width * (8 * m_height - 6) - 6 * m_height + 4;
  }

private:
  // @brief Converts pos_type_t to single index.
  constexpr size_type m_to_idx(pos_type_t pos) const noexcept {
    return pos.y * m_width + pos.x;
  }

  // @brief Converts single index to pos_type_t.
  constexpr pos_type_t m_to_pos(size_type idx) const noexcept {
    // @note %m_width is not allowed 0 -> noexcept.
    return {static_cast<diff_type>(idx % m_width),
            static_cast<diff_type>(idx / m_width)};
  }

  void m_set_tile(tile_type::value_type val, pos_type_t pos) noexcept {
    auto tile = m_get_tile(pos);
    if (tile)
      tile.value().get().value(val);
  }

  std::optional<std::reference_wrapper<tile_type>>
  m_get_tile(pos_type_t pos) noexcept {
    if (m_b_inside_bounds(pos))
      return std::optional<std::reference_wrapper<tile_type>>{
          m_tiles[m_to_idx(pos)]};
    return std::nullopt;
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
      m_tiles[no_mine_idx].value(tile_type::TILE_8);
      --m_mine_count;
      return;
    }
    // Random number generator for random mine positions.
    std::mt19937_64 rng(m_seed);
    for (size_type i = 0; i < mine_count; ++i) {
      auto idx = rng() % tile_count();
      if (idx != no_mine_idx && !m_tiles[idx].is_mine())
        m_tiles[idx].set_mine();
      // Reduce %i because the amount of mines won't change.
      else
        --i;
    }
  }

  // @brief Sets tiles without mines to have numbers representing how many
  // mines are nearby.
  void m_set_numbered_tiles() {
    for (auto i = m_next_mine(0); i < tile_count(); i = m_next_mine(i + 1)) {
      m_promote_tile(i - m_width);
      m_promote_tile(i + m_width);
      // If (index isn't against the right side wall). These indexes wrap
      // around the board to the otherside if %idx is next to the left side
      // wall.
      if (i % m_width != 0) {
        m_promote_tile(i - m_width - 1);
        m_promote_tile(i - 1);
        m_promote_tile(i + m_width - 1);
      }
      // If (index isn't against the right side wall). These indexes wrap
      // around the board to the otherside if %idx is next to the right side
      // wall.
      if (i % m_width != m_width - 1) {
        m_promote_tile(i - m_width + 1);
        m_promote_tile(i + 1);
        m_promote_tile(i + m_width + 1);
      }
    }
  }

  void m_set_numbered_tiles_pos() {
    for (auto p = m_next_mine({0, 0});
         pos_type_t::compare(p, {m_width, m_height}) == -1;
         p = m_next_mine(p + pos_type_t{1, 0})) {
      for (auto n : m_tile_neighbours_unbnds(p))
        m_promote_tile(n);
    }
  }

  // @brief Adds 1 to tile's value unless it's a mine. Empty tile changes
  // to 1. Does bound checking.
  void m_promote_tile(size_type idx) {
    if (m_b_inside_bounds(idx))
      m_tiles[idx].promote();
  }

  // @brief Adds 1 to tile's value unless it's a mine. Empty tile changes
  // to 1. Does bound checking.
  void m_promote_tile(pos_type_t pos) {
    auto tile = m_get_tile(pos);
    if (tile)
      tile.value().get().promote();
  }

  // @brief Checks that given index is inside the bounds of board size.
  constexpr bool m_b_inside_bounds(size_type index) const noexcept {
    return index < tile_count();
  }
  constexpr bool m_b_inside_bounds(pos_type_t pos) const noexcept {
    return pos.x >= 0 && pos.x < m_width && pos.y >= 0 && pos.y < m_height;
  }

  // @brief Returns next tile index with the type of mine starting from
  // optional index. If mine not found until the end of the array, returns the
  // maximum value of size_type.
  size_type m_next_mine(size_type idx) const noexcept {
    for (; idx < tile_count(); ++idx) {
      if (m_tiles[idx].is_mine())
        return idx;
    }
    return std::numeric_limits<size_type>::max();
  }

  // @brief Returns next tile position with the type of mine starting from
  // given position. If mine not found until the end of the array, returns
  // %pos_type_t with types' decltype(pos_type_t::x) and
  // decltype(pos_type_t::y) max values as parameters.
  pos_type_t m_next_mine(pos_type_t pos) noexcept {
    for (; pos_type_t::compare(pos, {m_width, m_height}) == -1;
         (pos.x % m_width != m_width - 1 ? pos += {1, 0}
                                         : pos += {-m_width + 1, 1})) {
      if (m_get_tile(pos) && m_get_tile(pos).value().get().is_mine())
        return pos;
    }
    return {std::numeric_limits<diff_type>::max(),
            std::numeric_limits<diff_type>::max()};
  }

  // @brief Returns the amount of neighbours tile has inside bounds of the
  // board.
  constexpr size_type m_neighbour_count(size_type idx) const noexcept {
    const bool vertical_edge =
                   idx % m_width == 0 || idx % m_width == m_width - 1,
               horizontal_edge =
                   idx < m_width || idx >= m_height * (m_width - 1);
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
               bottom_edge = idx < (tile_count() - m_width);
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

  std::vector<pos_type_t> m_tile_neighbours_bnds(pos_type_t pos) const {
    std::vector<pos_type_t> rv;
    const auto neigh_unbnds = m_tile_neighbours_unbnds(pos);
    for (const auto &unbnd : neigh_unbnds)
      if (m_b_inside_bounds(unbnd))
        rv.emplace_back(unbnd);
    return rv;
  }

  // @brief Takes a vector of neighbour indexes and does bound checking for
  // the contained indexes. Returns said vector as reference.
  std::vector<size_type> &
  m_tile_neighbours_bnds(std::vector<size_type> &neighbr_unbnds) const {
    for (size_type i = 0; i < neighbr_unbnds.size(); ++i)
      if (!m_b_inside_bounds(neighbr_unbnds[i]))
        neighbr_unbnds.erase(neighbr_unbnds.begin() + i);
    return neighbr_unbnds;
  }

  // @brief Returns a vector containing index's neighbours. Doesn't do bound
  // checking; check %m_tile_neighbours_bnds for that.
  // @note Returns as a vector because positions have to be partially bounds
  // checked and therefore output vector size cannot be determined at compile
  // time.
  std::vector<size_type> m_tile_neighbours_unbnds(size_type idx) const {
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

  static constexpr std::array<pos_type_t, TILE_NEIGHBOUR_COUNT>
  m_tile_neighbours_unbnds(pos_type_t pos) noexcept {
    return std::array<pos_type_t, TILE_NEIGHBOUR_COUNT>{
        {pos + pos_type_t{-1, -1}, pos + pos_type_t{0, -1},
         pos + pos_type_t{1, -1}, pos + pos_type_t{-1, 0},
         pos + pos_type_t{1, 0}, pos + pos_type_t{-1, 1},
         pos + pos_type_t{0, 1}, pos + pos_type_t{1, 1}}};
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
};

} // namespace rake

#endif