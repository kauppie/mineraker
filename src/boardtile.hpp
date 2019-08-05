#ifndef BOARDTILE_HPP
#define BOARDTILE_HPP

#include "mineraker.hpp"

namespace rake {
/**
 * Contains data of single tile placed on a board. Such information is: is the
 * tile open, is flag set on it and it's holding value which is either empty
 * represented by 0, number between 1-8 or mine represented by 9.
 */
class BoardTile {
public:
  using this_type = BoardTile;
  using value_type = unsigned char;
  using constant_type = const value_type;

private:
  // Defines tile's board value. Ranges from 0 to 9. Number 9 represents a mine.
  value_type tile_value : 6;
  // Defines if board tile is flagged.
  bool b_flagged : 1;
  // Defines if board tile is opened.
  bool b_open : 1;

public:
  static constant_type TILE_EMPTY = 0, TILE_1 = 1, TILE_2 = 2, TILE_3 = 3,
                       TILE_4 = 4, TILE_5 = 5, TILE_6 = 6, TILE_7 = 7,
                       TILE_8 = 8, TILE_MINE = 9;

  // @brief Default constructor that sets tile to cleared one.
  constexpr BoardTile()
      : tile_value(TILE_EMPTY), b_flagged(false), b_open(false) {}
  // @brief Constructor that takes tile value as parameter. Tile is set to
  // resetted one.
  constexpr BoardTile(value_type tile_value)
      : tile_value(tile_value), b_flagged(false), b_open(false) {}
  // @brief Constructor that takes all tile defining values as parameters and
  // constructs variables from them.
  constexpr BoardTile(value_type tile_value, bool is_flagged, bool is_open)
      : tile_value(tile_value), b_flagged(is_flagged), b_open(is_open) {}
  // @brief Copy constructor.
  constexpr BoardTile(const this_type &other)
      : tile_value(other.tile_value), b_flagged(other.b_flagged),
        b_open(other.b_open) {}
  // @brief Move constructor. Is deleted for usage of bit field member
  // variables.
  // constexpr BoardTile(this_type &&other) = delete;
  // @brief Deconstructor.
  ~BoardTile() noexcept {}

  // @brief Sets %tile_value to %new_value and returns %tile_value before
  // setting.
  auto value(value_type new_value) noexcept {
    auto tmp = tile_value;
    tile_value = new_value;
    return tmp;
  }
  // @brief Returns %tile_value.
  constexpr auto value() const noexcept { return tile_value; }
  // @brief Returns true when tile is flagged. False otherwise.
  constexpr auto is_flagged() const noexcept { return b_flagged; }
  // @brief Returns true when tile is open. False otherwise.
  constexpr auto is_open() const noexcept { return b_open; }
  // @brief Returns true when tile is a mine. False otherwise.
  constexpr bool is_mine() const noexcept { return tile_value == TILE_MINE; }
  // @brief Returns true when tile is empty. False otherwise.
  constexpr bool is_empty() const noexcept { return tile_value == TILE_EMPTY; }
  // @brief Returns true when tile is neither empty or a mine. False otherwise.
  constexpr bool is_number() const noexcept {
    return !(is_mine() || is_empty());
  }
  // @brief Sets tile to a mine.
  constexpr void set_mine() noexcept { tile_value = TILE_MINE; }
  // @brief Sets tile to empty tile.
  constexpr void set_empty() noexcept { tile_value = TILE_EMPTY; }
  // @brief Opens tile if tile is not flagged.
  constexpr void set_open() noexcept {
    if (!b_flagged)
      b_open = true;
  }
  // @brief Opens tile. Tile is set open whether or not tile is flagged.
  constexpr void set_open_unguarded() noexcept { b_open = true; }
  // @brief Closes tile.
  constexpr void set_closed() noexcept { b_open = false; }
  // @brief Sets tile flagged if tile is not open.
  constexpr void set_flagged() noexcept {
    if (!b_open)
      b_flagged = true;
  }
  // @brief Sets tile flagged. Tile is set flagged whether or not tile is
  // open.
  constexpr void set_flagged_unguarded() noexcept { b_flagged = true; }
  // @brief Sets tile unflagged.
  constexpr void set_unflagged() noexcept { b_flagged = false; }

  // @brief Toggles whether tile is flagged or not. Flagging is conditional in
  // the sense that opened tiles won't be flagged.
  constexpr void toggle_flag() noexcept {
    if (b_flagged)
      set_unflagged();
    else
      set_flagged();
  }

  // @brief Promotes tile to a higher value. Used when determining values for
  // non-mine positions on the board. Doesn't promote tiles valued 8 or mines.
  constexpr void promote() noexcept {
    if (tile_value < TILE_8)
      ++tile_value;
  }

  // @brief Clears BoardTile to have value of closed, unflagged and empty tile.
  constexpr void clear() noexcept {
    reset();
    set_empty();
  }

  // @brief Resets BoardTile to have value of closed and unflagged. Tile's board
  // value is left untouched.
  constexpr void reset() noexcept {
    set_closed();
    set_unflagged();
  }
};

} // namespace rake

#endif