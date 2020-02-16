#ifndef RAKE_MINEBOARDBASE_H
#define RAKE_MINEBOARDBASE_H

#include <stack>
#include <vector>

#include "boardtile.hpp"

namespace rake {

struct Position_t {
  using value_type = int32_t;
  value_type x, y;

  Position_t operator+(Position_t other) const {
    return {x + other.x, y + other.y};
  }
  Position_t operator-(Position_t other) const {
    return {x - other.x, y - other.y};
  }

  bool operator==(Position_t other) const {
    return y == other.y && x == other.x;
  }
  bool operator!=(Position_t other) const { return !(*this == other); }
  bool operator<(Position_t other) const {
    return y == other.y ? x < other.x : y < other.y;
  }
  bool operator>(Position_t other) const {
    return y == other.y ? x > other.x : y > other.y;
  }
  bool operator<=(Position_t other) const {
    return *this < other || *this == other;
  }
  bool operator>=(Position_t other) const {
    return *this > other || *this == other;
  }
};

class Mineboardbase {
public:
  using size_type = std::size_t;
  using diff_type = std::ptrdiff_t;

  using position_type = Position_t;
  using tile_type = BoardTile;

  static constexpr uint32_t TILE_NEIGHBOUR_COUNT = 8;

  Mineboardbase() : width_(0) {}
  Mineboardbase(size_type width, size_type height) : width_(0) {
    resize(width, height);
  }
  ~Mineboardbase() {}
  const tile_type& operator[](position_type pos) const { return at(pos); }
  tile_type& operator[](position_type pos) { return at(pos); }

  constexpr size_type width() const noexcept { return width_; }
  constexpr size_type height() const noexcept { return tiles_.size() / width_; }
  constexpr size_type tile_count() const noexcept { return tiles_.size(); }

  void resize(size_type width, size_type height) {
    tiles_.resize(width * height);
    width_ = width;
  }

  void open(position_type pos, bool open_by_flagged = false) {
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
  void open_single(position_type pos) { at(pos).set_open(); }

  std::vector<position_type> tile_neighbours(position_type pos) const {
    std::vector<position_type> neighbours;

    const std::array<position_type, TILE_NEIGHBOUR_COUNT> possible_neighbours{
        pos + position_type{-1, -1}, pos + position_type{0, -1},
        pos + position_type{1, -1},  pos + position_type{-1, 0},
        pos + position_type{1, 0},   pos + position_type{-1, 1},
        pos + position_type{0, 1},   pos + position_type{1, 1}};

    for (auto possible : possible_neighbours) {
      if (is_inside_bounds(possible)) {
        neighbours.push_back(possible);
      }
    }
    return neighbours;
  }

  std::vector<position_type> connected_emptys(position_type empty_pos) const {
    std::vector<position_type> emptys;
    // Check that %emptys_pos is valid and tile at %empty_pos is empty. Order
    // matters here!
    if (!is_inside_bounds(empty_pos) || !(*this)[empty_pos].is_empty()) {
      return emptys;
    }

    // Stack keeping track of possible emptys.
    std::stack<position_type> next_positions;
    // Vector keeping track of previously checked positions.
    std::vector<bool> checked_positions(tile_count(), false);

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
  const tile_type& at(position_type pos) const {
    return tiles_[pos_to_idx(pos)];
  }
  tile_type& at(position_type pos) { return tiles_[pos_to_idx(pos)]; }

  bool is_inside_bounds(position_type pos) const {
    return pos.x >= 0 && pos.y >= 0 && pos.x < width_ && pos.y < height();
  }

  inline size_type pos_to_idx(position_type pos) const noexcept {
    return pos.y * width_ + pos.x;
  }

  std::vector<tile_type> tiles_;
  size_type width_;
};

} // namespace rake

#endif
