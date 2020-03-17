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

#include "mineraker.hpp"
#include "tile.hpp"

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
  using tile_type = Tile;
  using this_type = MineBoard;
  using size_type = std::size_t;
  using diff_type = std::ptrdiff_t;

  using random_engine = std::mt19937_64;

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

    constexpr bool operator==(const pos_type& other) const {
      return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const pos_type& other) const {
      return x != other.x || y != other.y;
    }

    constexpr bool operator<(const pos_type& other) const {
      return compare(other) == -1;
    }

    constexpr bool operator>(const pos_type& other) const {
      return compare(other) == 1;
    }

    constexpr bool operator<=(const pos_type& other) const {
      return compare(other) != 1;
    }

    constexpr bool operator>=(const pos_type& other) const {
      return compare(other) != -1;
    }

    // @brief Compares this and other position types. First y is compared and if
    // not equal, return. Otherwise return comparison between x.
    constexpr int compare(const pos_type& other) const noexcept {
      auto comp = [](diff_type v1, diff_type v2) -> int {
        return (v1 < v2 ? -1 : (v2 < v1 ? 1 : 0));
      };
      auto ycomp = comp(y, other.y);
      return (ycomp != 0 ? ycomp : comp(x, other.x));
    }
  };

  static int compare(const pos_type& lhs, const pos_type& rhs) noexcept {
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

  static const size_type TILE_NEIGHBOUR_COUNT = 8;

private:
public:
  // Default container for the board tiles.
  std::vector<tile_type> tiles_;
  // Vector to store already opened empty tiles. Speeds up empty area opening
  // operations dramatically.
  std::vector<bool> opened_empty_tiles_;
  // Variable for storing the board width.
  size_type width_;
  // Variable for storing the board height.
  size_type height_;
  // Seed for mined tile position randomization.
  random_engine::result_type seed_;
  // Stores the amount of mines on the board.
  size_type mine_count_;
  // Represents current state of the board.
  State state_;

  // Adds control for the Control class. Might not be final.
  friend class GameManager;
  // Allows formatter to access private methods and variables.
  friend class MineBoardFormat;
  // Allows solver to access private information needed for solving the board.
  friend class MineBoardSolver;

public:
  // @brief Default constructor without parameters.
  MineBoard()
      : width_(0), height_(0), seed_(0), mine_count_(0), state_(UNINITIALIZED) {
  }
  MineBoard(const this_type& other)
      : tiles_(other.tiles_), opened_empty_tiles_(other.opened_empty_tiles_),
        width_(other.width_), height_(other.height_), seed_(other.seed_),
        mine_count_(other.mine_count_), state_(other.state_) {}
  MineBoard(this_type&& other) noexcept
      : tiles_(std::move(other.tiles_)),
        opened_empty_tiles_(std::move(other.opened_empty_tiles_)),
        width_(std::move(other.width_)), height_(std::move(other.height_)),
        seed_(std::move(other.seed_)),
        mine_count_(std::move(other.mine_count_)),
        state_(std::move(other.state_)) {}
  ~MineBoard() noexcept {}

  this_type& operator=(const this_type& other) {
    tiles_ = other.tiles_;
    opened_empty_tiles_ = other.opened_empty_tiles_;
    width_ = other.width_;
    height_ = other.height_;
    seed_ = other.seed_;
    mine_count_ = other.mine_count_;

    return *this;
  }

  this_type&& operator=(this_type&& other) noexcept {
    tiles_ = std::move(other.tiles_);
    opened_empty_tiles_ = std::move(other.opened_empty_tiles_);
    width_ = std::move(other.width_);
    height_ = std::move(other.height_);
    seed_ = std::move(other.seed_);
    mine_count_ = std::move(other.mine_count_);

    return std::move(*this);
  }

  void init(size_type width, size_type height, random_engine::result_type seed,
            size_type mine_count) {
    resize(width, height);
    m_clear();
    opened_empty_tiles_ = std::vector<bool>(tile_count(), false);
    seed_ = seed;
    mine_count_ = mine_count;
    state_ = FIRST_MOVE;
  }

  State open_tile(size_type idx) {
    if (state_ == UNINITIALIZED) {
      std::cerr << "\nMineBoard uninitialized!";
      return state_;
    } else if (!m_b_inside_bounds(idx) || tiles_[idx].is_flagged())
      return state_;
    switch (state_) {
    case NEXT_MOVE:
      m_on_next_move(idx);
      break;
    case FIRST_MOVE:
      m_on_first_move(idx);
      break;
    default:
      break;
    }
    return state_;
  }

  void m_on_next_move(size_type idx) {
    m_flood_open(idx);
    if (tile_count() - mine_count_ == open_tiles_count())
      state_ = GAME_WIN;
  }

  void m_on_first_move(size_type idx) {
    m_set_mines(mine_count_, idx);
    m_set_numbered_tiles();
    m_flood_open(idx);
    state_ = NEXT_MOVE;
  }

  void flag_tile(size_type idx) {
    if (m_b_inside_bounds(idx))
      tiles_[idx].toggle_flag();
  }

  void reset() { state_ = UNINITIALIZED; }

  // @brief Sets board dimensions and resizes the container.
  void resize(size_type width, size_type height) {
    try {
      tiles_.resize(width * height);
      opened_empty_tiles_.resize(width * height);
      width_ = width;
      height_ = height;
    } catch (std::exception& e) {
      std::cerr << "\nError: Couldn't reserve memory for mineboard: "
                << e.what();
    }
  }

  // @brief Sets the seed for the board generation.
  auto seed(random_engine::result_type seed) noexcept {
    auto old = seed_;
    seed_ = seed;
    return old;
  }

  // @brief Returns the set seed of the board.
  constexpr auto seed() const noexcept { return seed_; }

  // @brief Returns the state of the board.
  constexpr size_type state() const noexcept { return state_; }

  // @brief Returns the width of the board.
  constexpr size_type width() const noexcept { return width_; }

  // @brief Returns the height of the board.
  constexpr size_type height() const noexcept { return height_; }

  // @brief Returns the amount mines on the board.
  constexpr size_type mine_count() const noexcept { return mine_count_; }

  // @brief Returns the amount of tiles on the board.
  constexpr size_type tile_count() const noexcept { return width_ * height_; }

  // @brief Retunrs the amount of opened tiles on the board.
  size_type open_tiles_count() const noexcept {
    size_type open_tiles = 0;
    for (const auto& tile : tiles_)
      if (tile.is_open())
        ++open_tiles;
    return open_tiles;
  }

  // @brief Retunrs the amount of flagged tiles on the board.
  size_type flagged_tiles_count() const noexcept {
    size_type flagged_tiles = 0;
    for (const auto& tile : tiles_)
      if (tile.is_flagged())
        ++flagged_tiles;
    return flagged_tiles;
  }

  // @brief Returns the amount of neighbours tiles have combined.
  constexpr size_type neighbours_of_tiles_count() const noexcept {
    return width_ * (8 * height_ - 6) - 6 * height_ + 4;
  }

private:
  // @brief Converts pos_type to single index.
  constexpr size_type m_to_idx(pos_type pos) const noexcept {
    return pos.y * width_ + pos.x;
  }

  // @brief Converts single index to pos_type.
  constexpr pos_type m_to_pos(size_type idx) const {
    return {static_cast<diff_type>(idx % width_),
            static_cast<diff_type>(idx / width_)};
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
          tiles_[m_to_idx(pos)]};
    return std::nullopt;
  }

  tile_type* m_get_tile_ptr(pos_type pos) {
    if (m_b_inside_bounds(pos))
      return &tiles_[m_to_idx(pos)];
    return nullptr;
  }

  // @brief Sets every tile to an empty one.
  void m_clear() {
    for (auto& tile : tiles_)
      tile.clear();
    std::fill(opened_empty_tiles_.begin(), opened_empty_tiles_.end(), false);
  }

  // @brief Calculates mine count from given count and distributes them
  // evenly. %start_idx points to tile which won't filled by a mine.
  // @note Before calling, board must be empty of mines. Otherwise mine count
  // cannot be guaranteed.
  void m_set_mines(size_type mine_count, size_type start_idx) {
    // Make sure that mine count doesn't exceed board limits nor affect starting
    // area.
    mine_count_ =
        std::min(mine_count, tile_count() - m_neighbour_count(start_idx) - 1);

    // Random number generator for random mine positions.
    random_engine rng(seed_ + width_ + height_);

    // Vector of tiles that won't be filled with mines.
    auto empty_tiles = m_tile_neighbours_bnds(start_idx);
    empty_tiles.emplace_back(start_idx);

    // Loop until mines have been laid on the board.
    for (size_type mines_placed = 0; mines_placed < mine_count_;) {
      // Random index for placing a mine.
      auto idx = rng() % tile_count();

      if (!tiles_[idx].is_mine() &&
          std::find(empty_tiles.begin(), empty_tiles.end(), idx) ==
              empty_tiles.end()) {
        tiles_[idx].set_mine();
        ++mines_placed;
      }
    }
  }

  // @brief Sets tiles without mines to have numbers representing how many
  // mines are nearby.
  void m_set_numbered_tiles() {
    for (auto i = m_next_mine(0); i < tile_count(); i = m_next_mine(i + 1)) {
      m_promote_tile(i - width_);
      m_promote_tile(i + width_);
      // If (index isn't against the right side wall). These indexes wrap
      // around the board to the otherside if %idx is next to the left side
      // wall.
      if (i % width_ != 0) {
        m_promote_tile(i - width_ - 1);
        m_promote_tile(i - 1);
        m_promote_tile(i + width_ - 1);
      }
      // If (index isn't against the right side wall). These indexes wrap
      // around the board to the otherside if %idx is next to the right side
      // wall.
      if (i % width_ != width_ - 1) {
        m_promote_tile(i - width_ + 1);
        m_promote_tile(i + 1);
        m_promote_tile(i + width_ + 1);
      }
    }
  }

  void m_set_numbered_tiles_pos() {
    for (auto p = m_next_mine({0, 0});
         p.compare({static_cast<diff_type>(width_),
                    static_cast<diff_type>(height_)}) == -1;
         p = m_next_mine(p + pos_type{1, 0})) {
      for (auto n : m_tile_neighbours_unbnds(p))
        m_promote_tile(n);
    }
  }

  // @brief Adds 1 to tile's value unless it's a mine. Empty tile changes
  // to 1. Does bound checking.
  void m_promote_tile(size_type idx) {
    if (m_b_inside_bounds(idx))
      tiles_[idx].promote();
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
    return pos.x >= 0 && pos.x < static_cast<diff_type>(width_) && pos.y >= 0 &&
           pos.y < static_cast<diff_type>(height_);
  }

  // @brief Returns next tile index with the type of mine starting from
  // optional index. If mine not found until the end of the array, returns the
  // maximum value of size_type.
  auto m_next_mine(std::vector<tile_type>::iterator begin) const noexcept {
    for (; begin != tiles_.end(); ++begin) {
      if (begin->is_mine())
        return begin;
    }
    return tiles_.end();
  }

  // @brief Returns next tile position with the type of mine starting from
  // given position. If mine not found until the end of the array, returns
  // %pos_type with types' %diff_type: x and
  // %diff_type: y max values as parameters.
  pos_type m_next_mine(pos_type pos) noexcept {
    for (; pos.compare({static_cast<diff_type>(width_),
                        static_cast<diff_type>(height_)}) == -1;
         (pos.x % width_ != width_ - 1
              ? pos += {1, 0}
              : pos += {static_cast<diff_type>(-width_ + 1), 1})) {
      if (m_get_tile_ptr(pos)->is_mine())
        return pos;
    }
    return {std::numeric_limits<diff_type>::max(),
            std::numeric_limits<diff_type>::max()};
  }

  // @brief Returns the amount of neighbours tile has inside bounds of the
  // board.
  constexpr size_type m_neighbour_count(size_type idx) const noexcept {
    const bool vertical_edge = idx % width_ == 0 || idx % width_ == width_ - 1,
               horizontal_edge = idx < width_ || idx >= height_ * (width_ - 1);
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
    const bool up_edge = idx >= width_ && idx < tile_count(),
               bottom_edge = idx < (tile_count() - width_);
    if (up_edge)
      rv.emplace_back(idx - width_);
    if (bottom_edge)
      rv.emplace_back(idx + width_);
    // If (index isn't against the right side wall). These indexes wrap around
    // the board to the otherside if %idx is next to the left side wall.
    if (idx % width_ != 0) {
      if (up_edge)
        rv.emplace_back(idx - width_ - 1);
      rv.emplace_back(idx - 1);
      if (bottom_edge)
        rv.emplace_back(idx + width_ - 1);
    }
    // If (index isn't against the right side wall). These indexes wrap around
    // the board to the otherside if %idx is next to the right side wall.
    if (idx % width_ != width_ - 1) {
      if (up_edge)
        rv.emplace_back(idx - width_ + 1);
      rv.emplace_back(idx + 1);
      if (bottom_edge)
        rv.emplace_back(idx + width_ + 1);
    }
    return rv;
  }

  // @brief Returns bounds checked neighbours.
  void m_tile_neighbours_bnds(std::vector<size_type>& vec,
                              size_type idx) const {
    const bool up_edge = idx >= width_ && idx < tile_count(),
               bottom_edge = idx < (tile_count() - width_);
    if (up_edge)
      vec.emplace_back(idx - width_);
    if (bottom_edge)
      vec.emplace_back(idx + width_);
    // If (index isn't against the right side wall). These indexes wrap around
    // the board to the otherside if %idx is next to the left side wall.
    if (idx % width_ != 0) {
      if (up_edge)
        vec.emplace_back(idx - width_ - 1);
      vec.emplace_back(idx - 1);
      if (bottom_edge)
        vec.emplace_back(idx + width_ - 1);
    }
    // If (index isn't against the right side wall). These indexes wrap around
    // the board to the otherside if %idx is next to the right side wall.
    if (idx % width_ != width_ - 1) {
      if (up_edge)
        vec.emplace_back(idx - width_ + 1);
      vec.emplace_back(idx + 1);
      if (bottom_edge)
        vec.emplace_back(idx + width_ + 1);
    }
  }

  std::vector<pos_type> m_tile_neighbours_bnds(pos_type pos) const {
    std::vector<pos_type> rv;
    for (const auto& unbnd : m_tile_neighbours_unbnds(pos))
      if (m_b_inside_bounds(unbnd))
        rv.emplace_back(unbnd);
    return rv;
  }

  // @brief Takes a vector of neighbour indexes and does bound checking for
  // the contained indexes. Returns said vector as reference.
  void m_tile_neighbours_bnds(std::vector<size_type>& neighbr_unbnds) const {
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
    rv.emplace_back(idx - width_);
    rv.emplace_back(idx + width_);
    // If (index isn't against the right side wall). These indexes wrap around
    // the board to the otherside if %idx is next to the left side wall.
    if (idx % width_ != 0) {
      rv.emplace_back(idx - width_ - 1);
      rv.emplace_back(idx - 1);
      rv.emplace_back(idx + width_ - 1);
    }
    // If (index isn't against the right side wall). These indexes wrap around
    // the board to the otherside if %idx is next to the right side wall.
    if (idx % width_ != width_ - 1) {
      rv.emplace_back(idx - width_ + 1);
      rv.emplace_back(idx + 1);
      rv.emplace_back(idx + width_ + 1);
    }
    return rv;
  }

  std::array<pos_type, TILE_NEIGHBOUR_COUNT>&&
  m_tile_neighbours_unbnds(pos_type pos) const noexcept {
    return std::move(std::array<pos_type, TILE_NEIGHBOUR_COUNT>{
        {pos + pos_type{-1, -1}, pos + pos_type{0, -1}, pos + pos_type{1, -1},
         pos + pos_type{-1, 0}, pos + pos_type{1, 0}, pos + pos_type{-1, 1},
         pos + pos_type{0, 1}, pos + pos_type{1, 1}}});
  }

  void m_flood_open(size_type idx) {
    if (tiles_[idx].is_open()) {
      auto neighbrs = m_tile_neighbours_bnds(idx);
      size_type flagged_neighbrs = 0;
      for (auto i : neighbrs) {
        if (tiles_[i].is_flagged()) {
          ++flagged_neighbrs;
        }
      }
      if (flagged_neighbrs >= tiles_[idx].value()) {
        for (auto i : neighbrs) {
          m_open_single_tile(i);
          m_open_neighbours(m_empty_tiles_empty_area(i));
        }
      }
    } else {
      m_open_single_tile(idx);

      if (tiles_[idx].is_empty()) {
        m_open_neighbours(m_empty_tiles_empty_area(idx));
      }
    }
  }

  // @brief Returns vector of empty tiles that are neighbouring each other
  // starting from given index.
  std::vector<size_type> m_empty_tiles_empty_area(size_type idx) {
    if (!tiles_[idx].is_empty())
      return std::vector<size_type>{};
    if (opened_empty_tiles_[idx])
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
        if (tiles_[n].is_empty() && !tiles_[n].is_open() && !checked_tiles[n])
          st_neigh.emplace(n);
    }
    for (auto i : rv)
      opened_empty_tiles_[i] = true;
    return rv;
  }

  // @brief Takes vector of indexes and opens tiles' neighbouring tiles.
  void m_open_neighbours(const std::vector<size_type>& tiles) {
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
    if (tiles_[idx].is_flagged())
      return;
    if (tiles_[idx].is_mine())
      state_ = GAME_LOSE;
    tiles_[idx].set_open_unguarded();
  }
};

} // namespace rake

#endif