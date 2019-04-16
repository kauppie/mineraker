#ifndef BOARDTILE_HPP
#define BOARDTILE_HPP

namespace msgn {
/**
 * @brief class supporting activity and reducing complexity of SweepBoard.
 * Contains data of single tile placed on a board. Such information is: is the
 * tile open, is flag set on it and it's holding value which is either empty
 * represented by 0, number between 1-8 or mine represented by 9.
 *
 * Consumes 2 bytes compiled with g++. This could be size optimized to 1 byte,
 * but there could be greater loss when decoding wanted results.
 */
class BoardTile {
public:
  using constant_type = const unsigned char;
  using tile_value_type = unsigned char;

  // Defines if board tile is opened.
  bool b_open : 1;
  // Defines if board tile is flagged.
  bool b_flagged : 1;
  // Defines tile's board value. Ranges from 0 to 9. 9 represents a mine.
  tile_value_type tile_value;

  static constant_type TILE_EMPTY = 0, TILE_1 = 1, TILE_2 = 2, TILE_3 = 3,
                       TILE_4 = 4, TILE_5 = 5, TILE_6 = 6, TILE_7 = 7,
                       TILE_8 = 8, TILE_MINE = 9;

  // @brief Default constructor that sets tile to cleared one.
  BoardTile() : tile_value(TILE_EMPTY), b_flagged(false), b_open(false) {}
  // @brief Constructor that takes tile value as parameter. Tile is set to
  // resetted one.
  BoardTile(tile_value_type tile_value)
      : tile_value(tile_value), b_open(false), b_flagged(false) {}
  // @brief Constructor that takes all tile defining values as parameters and
  // constructs variables from them.
  BoardTile(tile_value_type tile_value, bool is_flagged, bool is_open)
      : tile_value(tile_value), b_flagged(is_flagged), b_open(is_open) {}
  // @brief Deconstructor.
  ~BoardTile() noexcept {}

  // @brief Returns true when tile is mine. False otherwise.
  bool is_mine() const noexcept { return tile_value == TILE_MINE; }
  // @brief Returns true when tile is empty. False otherwise.
  bool is_empty() const noexcept { return tile_value == TILE_EMPTY; }
  // @brief Sets tile to a mine.
  void set_mine() noexcept { tile_value = TILE_MINE; }
  // @brief Sets tile to a empty tile.
  void set_empty() noexcept { tile_value = TILE_EMPTY; }
  // @brief Opens tile.
  void set_open() noexcept { b_open = true; }
  // @brief Closes tile.
  void set_closed() noexcept { b_open = false; }
  // @brief Sets tile flagged.
  void set_flagged() noexcept { b_flagged = true; }
  // @brief Sets tile unflagged.
  void set_unflagged() noexcept { b_flagged = false; }

  // @brief Clears BoardTile to have value of closed, unflagged and empty tile.
  void clear() noexcept {
    set_closed();
    set_unflagged();
    set_empty();
  }

  // @brief Resets BoardTile to have value of closed and unflagged. Tile's board
  // value is left untouched.
  void reset() noexcept {
    set_closed();
    set_unflagged();
  }
}; // class BoardTile

} // namespace msgn

#endif