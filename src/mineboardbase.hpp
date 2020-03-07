#ifndef MINEBOARDBASE_HPP
#define MINEBOARDBASE_HPP

#include <algorithm>
#include <array>
#include <random>
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

  static constexpr size_type MAX_NEIGHBOUR_COUNT = 8;

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
   * @param pos Position wherefrom BoardTile is indexed.
   * @return constexpr const tile_type& constant reference to BoardTile.
   */
  tile_type at(pos_type pos) const { return tiles_[pos_to_idx(pos)]; }

  tile_type at(size_type idx) const { return tiles_[idx]; }

  tile_type& at(pos_type pos) { return tiles_[pos_to_idx(pos)]; }
  tile_type& at(size_type idx) { return tiles_[idx]; }

  /**
   * @brief Returns read-only (constant) iterator to the beginning of the vector
   * containing tiles.
   *
   * Forwards return value of std::vector::begin().
   */
  auto begin() const noexcept { return tiles_.begin(); }

  /**
   * @brief Returns read-only (constant) iterator to the end of the vector
   * containing tiles.
   *
   * Forwards return value of std::vector::end().
   */
  auto end() const noexcept { return tiles_.end(); }

  /**
   * @brief Returns width of the board.
   */
  constexpr pos_type::value_type width() const noexcept { return width_; }

  /**
   * @brief Returns height of the board.
   */
  pos_type::value_type height() const noexcept {
    return tiles_.size() / width_;
  }

  /**
   * @brief Returns the amount of tiles on board.
   *
   * Is equal to %width() * %height().
   */
  size_type size() const noexcept { return tiles_.size(); }

  /**
   * @brief Resize board.
   *
   * @param width Sets width for the board.
   * @param height Sets height for the board.
   */
  void resize(size_type width, size_type height) {
    tiles_.resize(width * height);
    width_ = width;
  }

  void clear() {
    auto s = tiles_.size();
    tiles_.clear();
    resize(width_, s / width_);
  }

  /**
   * @brief Generates new board with given amount of mines.
   *
   * @tparam RNG Random Number Generator type.
   * @param mines The amount of mines to be spread.
   * @param start_pos Starting position for the game. This position will be
   * left empty.
   * @param rng Random Number Generator. Used to randomize mine positions.
   * @param allow_mines_as_neighbours When set to true, all positions except
   * %start_pos will treated as possible mine positions. If false, neighbours
   * won't be filled with mines.
   */
  template<typename RNG>
  void generate(size_type mines, pos_type start_pos, RNG&& rng,
                bool allow_mines_as_neighbours = false) {
    // Calculate the amount of indexes that won't be considered possible mine
    // indexes.
    size_type left_untouched =
        allow_mines_as_neighbours ? 1 : neighbour_count(start_pos) + 1;
    // Calculate the amount of mines to spread, capped at total empty spots for
    // mines.
    size_type mines_to_spread = std::min(mines, size() - left_untouched);

    // Distribution for setting mines. Last indexes will be left empty to make
    // generation more efficient.
    std::uniform_int_distribution<size_type> dis(0,
                                                 size() - left_untouched - 1);

    // Spread mines on the board.
    for (size_type mines_spread = 0; mines_spread < mines_to_spread;) {
      // Generate new mine index.
      auto idx = dis(rng);

      if (!at(idx).is_mine()) {
        at(idx).set_mine();
        ++mines_spread;
      }
    }

    // Lambda for mapping "wrongly" positioned mines to the end of the vector.
    auto move_if_mine = [this](pos_type pos, size_type empty_idx) {
      tiles_[empty_idx].value(at(pos).value());
      at(pos).value(BoardTile::TILE_EMPTY);
    };

    // Map last empty coordinates to actual left empty coordinates.
    if (allow_mines_as_neighbours) {
      move_if_mine(start_pos, size() - 1);
    } else {
      auto empty_area = tile_neighbours(start_pos);
      empty_area.emplace_back(start_pos);

      for (size_type i = 0; i < empty_area.size(); ++i) {
        move_if_mine(empty_area[i], i + size() - left_untouched);
      }
    }

    // Set numbered tiles based on mine positions.
    set_numbered_tiles();
  }

  /**
   * @brief Open tile at given position.
   *
   * @param pos Selects the position for tile opening.
   * @param open_by_flagged Enables alternative opening style.
   *
   * Given that tile at %pos isn't yet
   * opened, on condition that the tile is empty, all of its
   * neighbours will be opened. If the opened tile is empty and has empty
   * neighbours, all connected neighbours will too be opened.
   * If %open_by_flagged is set true or left defaulted, 'opening' from opened
   * tile will open all neighbours if value of the tile is equal to the number
   * of flagged neighbours.
   */
  void open(pos_type pos, bool open_by_flagged = true) {
    if (!at(pos).is_open()) {
      auto emptys = connected_emptys(pos);

      for (auto empty_pos : emptys) {
        open_single(empty_pos);
      }
      open_single(pos);
    } else if (open_by_flagged) {
      auto neighbours = tile_neighbours(pos);

      auto flagged_neighbours =
          std::count_if(neighbours.begin(), neighbours.end(),
                        [this](pos_type pos) { return at(pos).is_flagged(); });

      if (flagged_neighbours == at(pos).value()) {
        std::for_each(neighbours.begin(), neighbours.end(),
                      [this](pos_type pos) { at(pos).set_open(); });
      }
    }
  }

  /**
   * @brief Flag the tile at given position.
   *
   * @param pos Position wherefrom tile is flagged.
   */
  void toggle_flag(pos_type pos) { at(pos).toggle_flag(); }

  /**
   * @brief
   *
   * @param pos
   * @return std::vector<pos_type>
   */
  std::vector<pos_type> tile_neighbours(pos_type pos) const {
    std::vector<pos_type> neighbours;
    neighbours.reserve(MAX_NEIGHBOUR_COUNT);

    // Positions around a given position.
    const std::array<pos_type, MAX_NEIGHBOUR_COUNT> possible_neighbours{
        pos + pos_type{-1, -1}, pos + pos_type{0, -1}, pos + pos_type{1, -1},
        pos + pos_type{-1, 0},  pos + pos_type{1, 0},  pos + pos_type{-1, 1},
        pos + pos_type{0, 1},   pos + pos_type{1, 1}};

    // Extract valid positions.
    std::copy_if(possible_neighbours.begin(), possible_neighbours.end(),
                 std::back_inserter(neighbours),
                 [this](const pos_type& pos) { return is_inside_bounds(pos); });
    return neighbours;
  }

  std::vector<pos_type> tile_neighbours(size_type idx) const {
    return tile_neighbours(idx_to_pos(idx));
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

  constexpr bool is_inside_bounds(pos_type pos) const noexcept {
    return pos.x >= 0 && pos.y >= 0 && pos.x < width_ && pos.y < height();
  }

private:
  /**
   * @brief Open single tile if not empty. If is, open it and neighbours.
   *
   * @param pos Position wherefrom tile is opened.
   */
  void open_single(pos_type pos) {
    auto& tile = at(pos);

    tile.set_open();
    if (tile.is_mine())
      set_state(LOSE);
    else if (tile.is_empty()) {
      auto neighbours = tile_neighbours(pos);
      std::for_each(neighbours.begin(), neighbours.end(),
                    [this](pos_type p) { at(p).set_open(); });
    }
  }

  void set_state(BoardState state) {
    // Setting new state to current state via map.
  }

  /**
   * @brief Set numbers for tiles based on mines' positions.
   *
   * Each mine is gone through its neighbouring tiles are promoted. When loop is
   * complete all tiles have correct numbers assigned to them.
   */
  void set_numbered_tiles() {
    for (size_type idx = 0; idx < size(); ++idx) {
      if (tiles_[idx].is_mine()) {
        auto neighbours = tile_neighbours(idx_to_pos(idx));
        std::for_each(neighbours.begin(), neighbours.end(),
                      [this](pos_type p) { at(p).promote(); });
      }
    }
  }

  constexpr size_type neighbour_count(pos_type pos) const noexcept {
    const bool vertical_edge = pos.x == 0 || pos.x == width_ - 1;
    const bool horizontal_edge = pos.y == 0 || pos.y == height() - 1;
    // If is against both vertically and horizontally going walls.
    if (vertical_edge && horizontal_edge)
      return 3;
    // If is against either vertically or horizontally going wall.
    if (vertical_edge || horizontal_edge)
      return 5;
    // If isn't against any walls.
    return MAX_NEIGHBOUR_COUNT;
  }

  constexpr size_type pos_to_idx(pos_type pos) const noexcept {
    return pos.y * width_ + pos.x;
  }

  /**
   * @brief Converts index to a position.
   *
   * @param idx Index to be converted to position.
   * @return constexpr pos_type Position to which index maps to.
   */
  constexpr pos_type idx_to_pos(size_type idx) const noexcept {
    return {static_cast<pos_type::value_type>(idx % width_),
            static_cast<pos_type::value_type>(idx / width_)};
  }

  std::vector<tile_type> tiles_;
  pos_type::value_type width_;
  BoardState state_;
};

} // namespace rake

#endif
