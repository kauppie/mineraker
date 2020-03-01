#ifndef MINEBOARDBASE_H
#define MINEBOARDBASE_H

#include <stack>
#include <vector>

#include "boardtile.hpp"

namespace rake {

enum BoardState {
  WIN,
  LOSE,
  PLAYING,
  READY,
  UNINITIALIZED,
  UNDEFINED,
};

struct Position_t {
  using value_type = int32_t;
  value_type x, y;

  constexpr Position_t operator+(Position_t other) const {
    return {x + other.x, y + other.y};
  }
  constexpr Position_t operator-(Position_t other) const {
    return {x - other.x, y - other.y};
  }
  constexpr bool operator==(Position_t other) const {
    return y == other.y && x == other.x;
  }
  constexpr bool operator!=(Position_t other) const {
    return !(*this == other);
  }
  constexpr bool operator<(Position_t other) const {
    return y == other.y ? x < other.x : y < other.y;
  }
  constexpr bool operator>(Position_t other) const {
    return y == other.y ? x > other.x : y > other.y;
  }
  constexpr bool operator<=(Position_t other) const {
    return *this < other || *this == other;
  }
  constexpr bool operator>=(Position_t other) const {
    return *this > other || *this == other;
  }
};

class Mineboardbase {
public:
  using size_type = std::size_t;
  using diff_type = std::ptrdiff_t;

  using pos_type = Position_t;
  using tile_type = BoardTile;

  static constexpr uint32_t TILE_NEIGHBOUR_COUNT = 8;

  explicit Mineboardbase() : tiles_(), width_(0), state_(UNINITIALIZED) {}
  Mineboardbase(const Mineboardbase& other) = default;
  Mineboardbase(Mineboardbase&& other) = default;
  Mineboardbase(size_type width, size_type height)
      : tiles_(), width_(0), state_(UNINITIALIZED) {
    resize(width, height);
  }
  ~Mineboardbase() {}

  /**
   * @brief Provide read-only access to parameter %pos defined %BoardTile.
   *
   * @param pos - Position wherefrom BoardTile is indexed.
   * @return constexpr const tile_type& - constant reference to BoardTile.
   */
  constexpr const tile_type& at(pos_type pos) const {
    return tiles_[pos_to_idx(pos)];
  }

  /**
   * @brief Returns read-only (constant) iterator to the beginning of the vector
   * containing tiles.
   *
   * Forwards return value of std::vector::begin().
   */
  constexpr auto begin() const noexcept { return tiles_.begin(); }

  /**
   * @brief Returns read-only (constant) iterator to the end of the vector
   * containing tiles.
   *
   * Forwards return value of std::vector::end().
   */
  constexpr auto end() const noexcept { return tiles_.end(); }

  /**
   * @brief Returns width of the board.
   */
  constexpr size_type width() const noexcept { return width_; }

  /**
   * @brief Returns height of the board.
   */
  constexpr size_type height() const noexcept { return tiles_.size() / width_; }

  /**
   * @brief Returns the amount of tiles on board.
   *
   * Is equal to %width() * %height().
   */
  constexpr size_type size() const noexcept { return tiles_.size(); }

  /**
   * @brief Resize board.
   *
   * @param width - Sets width for the board.
   * @param height - Sets height for the board.
   */
  void resize(size_type width, size_type height) {
    tiles_.resize(width * height);
    width_ = width;
  }

  template<typename RNG> void generate(size_type mines, RNG&& rng) {
    std::shuffle(tiles_.begin(), tiles_.end(), rng);
  }

  /**
   * @brief Open tile at given position.
   *
   * @param pos - Selects the position for tile opening.
   * @param open_by_flagged - Enables alternative opening style.
   *
   * Given that tile at %pos isn't yet
   * opened, the tile, and on condition that the tile is empty, all of its
   * neighbours will be opened. If the opened tile is empty and has empty
   * neighbours, all connected neighbours will too be opened.
   * If %open_by_flagged is set true or left defaulted, 'opening' from opened
   * tile will open all neighbours if value of the tile is equal to the number
   * of flagged neighbours.
   */
  void open(pos_type pos, bool open_by_flagged = true) {
    if (!at(pos).is_open()) {
      auto emptys = connected_emptys(pos);

      if (!emptys.empty()) {
        for (auto empty_pos : emptys) {
          for (auto empty_neighbour : tile_neighbours(empty_pos)) {
            open_single(empty_neighbour);
          }
        }
      }
      open_single(pos);
    } else if (open_by_flagged) {
      auto neighbours = tile_neighbours(pos);

      auto flagged_neighbours = std::count_if(
          neighbours.begin(), neighbours.end(), tile_type::is_flagged);

      if (flagged_neighbours == at(pos).value()) {
        std::for_each(neighbours.begin(), neighbours.end(),
                      tile_type::set_open);
      }
    }
  }

  /**
   * @brief Flag the tile at given position.
   *
   * @param pos - Position wherefrom tile is flagged.
   */
  constexpr void toggle_flag(pos_type pos) { at(pos).toggle_flag(); }

  /**
   * @brief
   *
   * @param pos
   * @return std::vector<pos_type>
   */
  std::vector<pos_type> tile_neighbours(pos_type pos) const {
    std::vector<pos_type> neighbours;
    neighbours.reserve(TILE_NEIGHBOUR_COUNT);

    const std::array<pos_type, TILE_NEIGHBOUR_COUNT> possible_neighbours{
        pos + pos_type{-1, -1}, pos + pos_type{0, -1}, pos + pos_type{1, -1},
        pos + pos_type{-1, 0},  pos + pos_type{1, 0},  pos + pos_type{-1, 1},
        pos + pos_type{0, 1},   pos + pos_type{1, 1}};

    std::copy_if(possible_neighbours.begin(), possible_neighbours.end(),
                 std::back_inserter(neighbours), is_inside_bounds);
    return neighbours;
  }

  std::vector<pos_type> connected_emptys(pos_type empty_pos) const {
    std::vector<pos_type> emptys;
    // Check that %emptys_pos is valid and tile at %empty_pos is empty. Order
    // matters here!
    if (!is_inside_bounds(empty_pos) || !at(empty_pos).is_empty()) {
      return emptys;
    }

    // Stack keeping track of possible emptys.
    std::stack<pos_type> next_positions;
    // Vector keeping track of previously checked positions.
    std::vector<bool> checked_positions(size(), false);

    next_positions.emplace(empty_pos);
    while (!next_positions.empty()) {
      auto empty = next_positions.top();
      next_positions.pop();

      // Add checked empty.
      emptys.emplace_back(empty);
      checked_positions[pos_to_idx(empty)] = true;

      // Find new emptys neighbouring the checked empty.
      for (auto neighbour : tile_neighbours(empty)) {
        // Check that neighbouring tile is empty and hasn't yet been added to
        // the vector.
        if (at(neighbour).is_empty() &&
            !checked_positions[pos_to_idx(neighbour)]) {
          next_positions.emplace(neighbour);
        }
      }
    }
    return emptys;
  }

private:
  constexpr tile_type& at(pos_type pos) { return tiles_[pos_to_idx(pos)]; }

  constexpr void open_single(pos_type pos) {
    at(pos).set_open();
    if (at(pos).is_mine())
      set_state(LOSE);
  }

  void set_state(BoardState state) {
    // Setting new state to current state via map.
  }

  constexpr bool is_inside_bounds(pos_type pos) const noexcept {
    return pos.x >= 0 && pos.y >= 0 && pos.x < width_ && pos.y < height();
  }

  constexpr size_type pos_to_idx(pos_type pos) const noexcept {
    return pos.y * width_ + pos.x;
  }

  std::vector<tile_type> tiles_;
  pos_type::value_type width_;
  BoardState state_;
};

} // namespace rake

#endif
