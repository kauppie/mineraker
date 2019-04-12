
namespace msgn {
/**
 * @brief class supporting activity and reducing complexity of SweepBoard.
 * Contains data of single tile placed on a board. Such information is: is the
 * tile open, is flag set on it and it's holding value which is either empty
 * represented by 0, number between 1-8 or mine represented by 9.
 */
class BoardTile {
public:
  using constant_type = const unsigned char;
  using tile_value_type = unsigned char;

  bool b_open : 1;
  bool b_flagged : 1;
  tile_value_type tile_value;
  static constant_type TILE_EMPTY = 0, TILE_1 = 1, TILE_2 = 2, TILE_3 = 3,
                       TILE_4 = 4, TILE_5 = 5, TILE_6 = 6, TILE_7 = 7,
                       TILE_8 = 8, TILE_MINE = 9;

  BoardTile() : tile_value(TILE_EMPTY), b_flagged(false), b_open(false) {}
  BoardTile(tile_value_type tile_value_) : tile_value(tile_value_) {}
  BoardTile(tile_value_type tile_value_, bool is_flagged, bool is_open)
      : tile_value(tile_value_), b_flagged(is_flagged), b_open(is_open) {}
  ~BoardTile() noexcept {}

  bool is_mine() const { return tile_value == TILE_MINE; }
  bool is_empty() const { return tile_value == TILE_EMPTY; }
  void set_mine() { tile_value = TILE_MINE; }
  void set_empty() { tile_value = TILE_EMPTY; }
  void set_open() { b_open = true; }
  void set_closed() { b_open = false; }
  void set_flagged() { b_flagged = true; }
  void set_unflagged() { b_flagged = false; }

  void reset() {
    set_closed();
    set_unflagged();
    set_empty();
  }
}; // class BoardTile

} // namespace msgn