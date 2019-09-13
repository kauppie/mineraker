#ifndef MINEBOARD_HPP
#define MINEBOARD_HPP

#include <algorithm>
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

  struct pos_type {
    diff_type x, y;

    // @brief Addition.
    constexpr pos_type operator+(pos_type other) const noexcept {
      return {x + other.x, y + other.y};
    }

    // @brief Substraction.
    constexpr pos_type operator-(pos_type other) const noexcept {
      return {x - other.x, y - other.y};
    }

    // @brief Unary plus.
    constexpr pos_type operator+() const noexcept { return *this; }

    // @brief Unary minus.
    constexpr pos_type operator-() const noexcept { return {-x, -y}; }

    // @brief Addition assignment operator implementation.
    constexpr pos_type operator+=(pos_type other) noexcept {
      *this = {x + other.x, y + other.y};
      return *this;
    }

    // @brief Substraction assignment operator implementation.
    constexpr pos_type operator-=(pos_type other) noexcept {
      *this = {x - other.x, y - other.y};
      return *this;
    }

    constexpr bool operator==(const pos_type &other) const {
      return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const pos_type &other) const {
      return x != other.x || y != other.y;
    }

    constexpr bool operator<(const pos_type &other) const {
      return compare(other) == -1;
    }

    constexpr bool operator>(const pos_type &other) const {
      return compare(other) == 1;
    }

    constexpr bool operator<=(const pos_type &other) const {
      return compare(other) != 1;
    }

    constexpr bool operator>=(const pos_type &other) const {
      return compare(other) != -1;
    }

    // @brief Compares this and other position types. First y is compared and if
    // not equal, return. Otherwise return comparison between x.
    constexpr int compare(const pos_type &other) const noexcept {
      auto comp = [](diff_type v1, diff_type v2) -> int {
        return (v1 < v2 ? -1 : (v2 < v1 ? 1 : 0));
      };
      auto ycomp = comp(y, other.y);
      return (ycomp != 0 ? ycomp : comp(x, other.x));
    }
  };

  static int compare(const pos_type &lhs, const pos_type &rhs) noexcept {
    auto comp = [](diff_type v1, diff_type v2) -> int {
      return (v1 < v2 ? -1 : (v2 < v1 ? 1 : 0));
    };
    auto ycomp = comp(lhs.y, rhs.y);
    return (ycomp != 0 ? ycomp : comp(lhs.x, rhs.x));
  }

  enum State {
    ERROR,
    UNINITIALIZED,
    FIRST_MOVE,
    NEXT_MOVE,
    GAME_WIN,
    GAME_LOSE,
  };

  static const unsigned char TILE_NEIGHBOUR_COUNT = 8;

private:
  // Default container for the board tiles.
  std::vector<tile_type> m_tiles;
  // Vector to store already opened empty tiles. Speeds up empty area opening
  // operations dramatically.
  std::vector<bool> m_opened_empty_tiles;
  // Variable for storing the board width.
  size_type m_width;
  // Variable for storing the board height.
  size_type m_height;
  // Seed for mined tile position randomization.
  std::mt19937_64::result_type m_seed;
  // Stores the amount of mines on the board.
  size_type m_mine_count;
  // Represents current state of the board.
  State m_state;

  // Adds control for the Control class. Might not be final.
  friend class GameManager;
  // Allows formatter to access private methods and variables.
  friend class MineBoardFormat;
  // Allows solver to access private information needed for solving the board.
  friend class MineBoardSolver;

public:
  // @brief Default constructor without parameters.
  MineBoard()
      : m_width(0), m_height(0), m_seed(0), m_mine_count(0),
        m_state(UNINITIALIZED) {}
  MineBoard(const this_type &other)
      : m_tiles(other.m_tiles),
        m_opened_empty_tiles(other.m_opened_empty_tiles),
        m_width(other.m_width), m_height(other.m_height), m_seed(other.m_seed),
        m_mine_count(other.m_mine_count), m_state(other.m_state) {}
  MineBoard(this_type &&other) noexcept
      : m_tiles(std::move(other.m_tiles)),
        m_opened_empty_tiles(std::move(other.m_opened_empty_tiles)),
        m_width(std::move(other.m_width)), m_height(std::move(other.m_height)),
        m_seed(std::move(other.m_seed)),
        m_mine_count(std::move(other.m_mine_count)),
        m_state(std::move(other.m_state)) {}
  ~MineBoard() noexcept {}

  this_type &operator=(const this_type &other) {
    m_tiles = other.m_tiles;
    m_opened_empty_tiles = other.m_opened_empty_tiles;
    m_width = other.m_width;
    m_height = other.m_height;
    m_seed = other.m_seed;
    m_mine_count = other.m_mine_count;

    return *this;
  }

  this_type &&operator=(this_type &&other) noexcept {
    m_tiles = std::move(other.m_tiles);
    m_opened_empty_tiles = std::move(other.m_opened_empty_tiles);
    m_width = std::move(other.m_width);
    m_height = std::move(other.m_height);
    m_seed = std::move(other.m_seed);
    m_mine_count = std::move(other.m_mine_count);

    return std::move(*this);
  }

  void init(size_type width, size_type height,
            std::mt19937_64::result_type seed, size_type mine_count) {
    resize(width, height);
    m_clear();
    m_opened_empty_tiles = std::vector<bool>(tile_count(), false);
    m_seed = seed;
    m_mine_count = mine_count;
    m_state = FIRST_MOVE;
  }

  State open_tile(size_type idx) {
    if (m_state == UNINITIALIZED) {
      std::cerr << "\nMineBoard uninitialized!";
      return m_state;
    } else if (!m_b_inside_bounds(idx) || m_tiles[idx].is_flagged())
      return m_state;
    switch (m_state) {
    case NEXT_MOVE:
      m_on_next_move(idx);
      break;
    case FIRST_MOVE:
      m_on_first_move(idx);
      break;
    default:
      break;
    }
    return m_state;
  }

  void m_on_next_move(size_type idx) {
    m_flood_open(idx);
    if (tile_count() - m_mine_count == open_tiles_count())
      m_state = GAME_WIN;
  }

  void m_on_first_move(size_type idx) {
    m_set_mines(m_mine_count, idx);
    m_set_numbered_tiles();
    m_flood_open(idx);
    m_state = NEXT_MOVE;
  }

  void flag_tile(size_type idx) {
    if (m_b_inside_bounds(idx))
      m_tiles[idx].toggle_flag();
  }

  void reset() { m_state = UNINITIALIZED; }

  // @brief Sets board dimensions and resizes the container.
  void resize(size_type width, size_type height) {
    try {
      m_tiles.resize(width * height);
      m_opened_empty_tiles.resize(width * height);
      m_width = width;
      m_height = height;
    } catch (std::exception &e) {
      std::cerr << "\nError: Couldn't reserve memory for mineboard: "
                << e.what();
    }
  }

  auto seed(std::mt19937_64::result_type seed) noexcept {
    auto old = m_seed;
    m_seed = seed;
    return old;
  }

  constexpr auto seed() const noexcept { return m_seed; }

  constexpr size_type state() const noexcept { return m_state; }

  // @brief Returns the width of the board.
  constexpr size_type width() const noexcept { return m_width; }

  // @brief Returns the height of the board.
  constexpr size_type height() const noexcept { return m_height; }

  // @brief Returns the amount mines on the board.
  constexpr size_type mine_count() const noexcept { return m_mine_count; }

  // @brief Returns the amount of tiles on the board.
  constexpr size_type tile_count() const noexcept { return m_width * m_height; }

  // @brief Retunrs the amount of opened tiles on the board.
  size_type open_tiles_count() const noexcept {
    size_type open_tiles = 0;
    for (const auto &tile : m_tiles)
      if (tile.is_open())
        ++open_tiles;
    return open_tiles;
  }

  // @brief Retunrs the amount of flagged tiles on the board.
  size_type flagged_tiles_count() const noexcept {
    size_type flagged_tiles = 0;
    for (const auto &tile : m_tiles)
      if (tile.is_flagged())
        ++flagged_tiles;
    return flagged_tiles;
  }

  // @brief Returns the amount of neighbours tiles have combined.
  constexpr size_type neighbours_of_tiles_count() const noexcept {
    return m_width * (8 * m_height - 6) - 6 * m_height + 4;
  }

private:
  // @brief Converts pos_type to single index.
  constexpr size_type m_to_idx(pos_type pos) const noexcept {
    return pos.y * m_width + pos.x;
  }

  // @brief Converts single index to pos_type.
  constexpr pos_type m_to_pos(size_type idx) const {
    return {static_cast<diff_type>(idx % m_width),
            static_cast<diff_type>(idx / m_width)};
  }

  void m_set_tile(tile_type::value_type val, pos_type pos) noexcept {
    auto tile = m_get_tile(pos);
    if (tile)
      tile.value().get().value(val);
  }

  void m_set_tile_ptr(tile_type::value_type val, pos_type pos) noexcept {
    auto tile = m_get_tile_ptr(pos);
    if (tile != nullptr)
      tile->value(val);
  }

  std::optional<std::reference_wrapper<tile_type>>
  m_get_tile(pos_type pos) noexcept {
    if (m_b_inside_bounds(pos))
      return std::optional<std::reference_wrapper<tile_type>>{
          m_tiles[m_to_idx(pos)]};
    return std::nullopt;
  }

  tile_type *m_get_tile_ptr(pos_type pos) {
    if (m_b_inside_bounds(pos))
      return &m_tiles[m_to_idx(pos)];
    return nullptr;
  }

  // @brief Sets every tile to an empty one.
  void m_clear() {
    for (auto &tile : m_tiles)
      tile.clear();
    std::fill(m_opened_empty_tiles.begin(), m_opened_empty_tiles.end(), false);
  }

  // @brief Calculates mine count from given count and distributes them
  // evenly. %start_idx points to tile which won't filled by a mine.
  // @note Before calling, board must be empty of mines. Otherwise mine count
  // cannot be guaranteed.
  void m_set_mines(size_type mine_count, size_type start_idx) {
    // Make sure that mine count doesn't exceed board limits nor affect starting
    // area.
    m_mine_count =
        std::min(mine_count, tile_count() - m_neighbour_count(start_idx) - 1);

    // Random number generator for random mine positions.
    std::mt19937_64 rng(m_seed + m_width + m_height);

    // Vector of tiles that won't be filled with mines.
    auto empty_tiles = m_tile_neighbours_bnds(start_idx);
    empty_tiles.emplace_back(start_idx);

    // Loop until mines have been laid on the board.
    for (size_type i = 0; i < m_mine_count; ++i) {
      // Random index for placing a mine.
      auto idx = rng() % tile_count();
      if (!m_tiles[idx].is_mine()) {
        bool set_mine = true;
        // Ensure that %idx isn't one of the tiles not to be filled.
        for (auto empty_idx : empty_tiles) {
          if (idx == empty_idx) {
            set_mine = false;
            --i;
          }
        }
        if (set_mine)
          m_tiles[idx].set_mine();
      }
      // Reduce %i because the amount of mines haven't changed.
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
         p.compare({static_cast<diff_type>(m_width),
                    static_cast<diff_type>(m_height)}) == -1;
         p = m_next_mine(p + pos_type{1, 0})) {
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
  void m_promote_tile(pos_type pos) {
    auto tile = m_get_tile(pos);
    if (tile)
      tile.value().get().promote();
  }

  void m_promote_tile_ptr(pos_type pos) {
    auto tile = m_get_tile_ptr(pos);
    if (tile != nullptr)
      tile->promote();
  }

  // @brief Checks that given index is inside the bounds of board size.
  constexpr bool m_b_inside_bounds(size_type index) const noexcept {
    return index < tile_count();
  }
  constexpr bool m_b_inside_bounds(pos_type pos) const noexcept {
    return pos.x >= 0 && pos.x < m_width && pos.y >= 0 && pos.y < m_height;
  }

  // @brief Returns next tile index with the type of mine starting from
  // optional index. If mine not found until the end of the array, returns the
  // maximum value of size_type.
  size_type m_next_mine(size_type offset) const noexcept {
    for (; offset < tile_count(); ++offset) {
      if (m_tiles[offset].is_mine())
        return offset;
    }
    return std::numeric_limits<size_type>::max();
  }

  // @brief Returns next tile position with the type of mine starting from
  // given position. If mine not found until the end of the array, returns
  // %pos_type with types' %diff_type: x and
  // %diff_type: y max values as parameters.
  pos_type m_next_mine(pos_type pos) noexcept {
    for (; pos.compare({static_cast<diff_type>(m_width),
                        static_cast<diff_type>(m_height)}) == -1;
         (pos.x % m_width != m_width - 1
              ? pos += {1, 0}
              : pos += {static_cast<diff_type>(-m_width + 1), 1})) {
      if (m_get_tile_ptr(pos)->is_mine())
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
    rv.reserve(m_neighbour_count(idx));
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

  // @brief Returns bounds checked neighbours.
  void m_tile_neighbours_bnds(std::vector<size_type> &vec,
                              size_type idx) const {
    const bool up_edge = idx >= m_width && idx < tile_count(),
               bottom_edge = idx < (tile_count() - m_width);
    if (up_edge)
      vec.emplace_back(idx - m_width);
    if (bottom_edge)
      vec.emplace_back(idx + m_width);
    // If (index isn't against the right side wall). These indexes wrap around
    // the board to the otherside if %idx is next to the left side wall.
    if (idx % m_width != 0) {
      if (up_edge)
        vec.emplace_back(idx - m_width - 1);
      vec.emplace_back(idx - 1);
      if (bottom_edge)
        vec.emplace_back(idx + m_width - 1);
    }
    // If (index isn't against the right side wall). These indexes wrap around
    // the board to the otherside if %idx is next to the right side wall.
    if (idx % m_width != m_width - 1) {
      if (up_edge)
        vec.emplace_back(idx - m_width + 1);
      vec.emplace_back(idx + 1);
      if (bottom_edge)
        vec.emplace_back(idx + m_width + 1);
    }
  }

  std::vector<pos_type> m_tile_neighbours_bnds(pos_type pos) const {
    std::vector<pos_type> rv;
    for (const auto &unbnd : m_tile_neighbours_unbnds(pos))
      if (m_b_inside_bounds(unbnd))
        rv.emplace_back(unbnd);
    return rv;
  }

  // @brief Takes a vector of neighbour indexes and does bound checking for
  // the contained indexes. Returns said vector as reference.
  void m_tile_neighbours_bnds(std::vector<size_type> &neighbr_unbnds) const {
    for (size_type i = 0; i < neighbr_unbnds.size(); ++i)
      if (!m_b_inside_bounds(neighbr_unbnds[i]))
        neighbr_unbnds.erase(neighbr_unbnds.begin() + i);
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

  static constexpr std::array<pos_type, TILE_NEIGHBOUR_COUNT>
  m_tile_neighbours_unbnds(pos_type pos) noexcept {
    return std::array<pos_type, TILE_NEIGHBOUR_COUNT>{
        {pos + pos_type{-1, -1}, pos + pos_type{0, -1}, pos + pos_type{1, -1},
         pos + pos_type{-1, 0}, pos + pos_type{1, 0}, pos + pos_type{-1, 1},
         pos + pos_type{0, 1}, pos + pos_type{1, 1}}};
  }

  void m_flood_open(size_type idx) {
    if (m_tiles[idx].is_open()) {
      auto neighbrs = m_tile_neighbours_bnds(idx);
      size_type flagged_neighbrs = 0;
      for (auto i : neighbrs)
        if (m_tiles[i].is_flagged())
          ++flagged_neighbrs;
      if (flagged_neighbrs >= m_tiles[idx].value())
        for (auto i : neighbrs) {
          m_open_single_tile(i);
          m_open_neighbours(m_empty_tiles_empty_area(i));
        }
    } else {
      m_open_single_tile(idx);
      if (m_tiles[idx].is_empty())
        m_open_neighbours(m_empty_tiles_empty_area(idx));
    }
  }

  // @brief Returns vector of empty tiles that are neighbouring each other
  // starting from given index.
  std::vector<size_type> m_empty_tiles_empty_area(size_type idx) {
    if (!m_tiles[idx].is_empty())
      return std::vector<size_type>{};
    if (m_opened_empty_tiles[idx])
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
    for (auto i : rv)
      m_opened_empty_tiles[i] = true;
    return rv;
  }

  // @brief Takes vector of indexes and opens tiles' neighbouring tiles.
  void m_open_neighbours(const std::vector<size_type> &tiles) {
    std::vector<size_type> to_open_tiles;
    for (auto idx : tiles) {
      auto neighbrs_bnds = m_tile_neighbours_bnds(idx);
      to_open_tiles.insert(to_open_tiles.end(), neighbrs_bnds.begin(),
                           neighbrs_bnds.end());
    }
    for (auto idx : to_open_tiles)
      m_open_single_tile(idx);
  }

  void m_open_single_tile(size_type idx) {
    if (m_tiles[idx].is_flagged())
      return;
    if (m_tiles[idx].is_mine())
      m_state = GAME_LOSE;
    m_tiles[idx].set_open_unguarded();
  }
};

} // namespace rake

#endif