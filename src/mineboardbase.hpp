#ifndef RAKE_MINEBOARDBASE_H
#define RAKE_MINEBOARDBASE_H

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

  Mineboardbase() : tiles_(), width_(0) {}
  ~Mineboardbase() {}

  tile_type& operator[](position_type pos) noexcept {
    return tiles_[pos.y * width_ + pos.x];
  }
  tile_type& at(position_type pos) { return tiles_.at(pos.y * width_ + pos.x); }

  size_type width() const { return width_; }
  size_type height() const { return tiles_.size() / width_; }

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

private:
  bool is_inside_bounds(position_type pos) const {
    return pos.x >= 0 && pos.y >= 0 && pos.x < width_ && pos.y < height();
  }

  std::vector<tile_type> tiles_;
  size_type width_;
};

} // namespace rake

#endif
