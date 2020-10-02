#ifndef TILE_HPP
#define TILE_HPP

#include <cstdint>

namespace rake {
/**
 * Contains data of a single tile placed on a board. Such information is: is the
 * tile open, is flag set on it and its holding value which is either empty
 * represented by 0, number between 1-8 or mine represented by 9.
 */
class Tile {
 public:
  using this_type = Tile;
  using value_type = uint8_t;

 private:
  // Defines tile's board value. Ranges from 0 to 9. Number 9 represents a mine.
  value_type m_tile_value : 6;
  // Defines if board tile is flagged.
  bool m_b_flagged : 1;
  // Defines if board tile is opened.
  bool m_b_open : 1;

 public:
  enum {
    TILE_EMPTY = 0,
    TILE_1 = 1,
    TILE_2 = 2,
    TILE_3 = 3,
    TILE_4 = 4,
    TILE_5 = 5,
    TILE_6 = 6,
    TILE_7 = 7,
    TILE_8 = 8,
    TILE_MINE = 9
  };

  // @brief Default constructor that sets tile to cleared one.
  constexpr Tile()
      : m_tile_value(TILE_EMPTY), m_b_flagged(false), m_b_open(false) {}
  // @brief Constructor that takes tile value as parameter. Tile is set to
  // resetted one.
  constexpr Tile(value_type m_tile_value)
      : m_tile_value(m_tile_value), m_b_flagged(false), m_b_open(false) {}
  // @brief Constructor that takes all tile defining values as parameters and
  // constructs variables from them.
  constexpr Tile(value_type m_tile_value, bool is_flagged, bool is_open)
      : m_tile_value(m_tile_value),
        m_b_flagged(is_flagged),
        m_b_open(is_open) {}
  // @brief Copy constructor.
  constexpr Tile(const this_type &other)
      : m_tile_value(other.m_tile_value),
        m_b_flagged(other.m_b_flagged),
        m_b_open(other.m_b_open) {}
  // Can't use rvalue references on bit fields.
  Tile(this_type &&other) = delete;
  // @brief Move constructor. Is deleted for usage of bit field member
  // variables.
  // @brief Deconstructor.
  ~Tile() noexcept {}

  /**
   * @brief Copies data from %other to *this.
   * @param other const reference to other Tile. Source of data to copy.
   * @return Reference to *this.
   * @pre -
   * @post *this is an exact copy of %other.
   * @exception No-throw guarantee.
   */
  constexpr this_type &operator=(const this_type &other) noexcept {
    m_tile_value = other.m_tile_value;
    m_b_flagged = other.m_b_flagged;
    m_b_open = other.m_b_open;

    return *this;
  }

  /// @brief Deleted method.
  constexpr this_type &operator=(this_type &&other) = delete;

  /**
   * @brief Sets value to parameter %new_value.
   * @param new_value New value to be assigned to.
   * @pre -
   * @post Value of *this is equal to given parameter.
   * @exception No-throw guarantee.
   */
  constexpr void set_value(value_type new_value) noexcept {
    m_tile_value = new_value;
  }

  /**
   * @brief Returns value of the tile.
   * @return Value of *this.
   * @pre Return value is undefined if value is yet to be assigned.
   * @post -
   * @exception No-throw guarantee.
   */
  constexpr auto value() const noexcept { return m_tile_value; }

  /**
   * @brief Returns true if the Tile is flagged, false otherwise.
   * @return Flag state of the Tile. True if flagged, false if not.
   * @pre Return value is undefined if flagged state is yet to be assigned.
   * @post -
   * @exception No-throw guarantee.
   */
  constexpr auto is_flagged() const noexcept { return m_b_flagged; }

  /**
   * @brief Return true if the Tile is open, false otherwise.
   * @return Open state of the Tile. True if open, false if closed.
   * @pre Return value is undefined if open state is yet to be assigned.
   * @post -
   * @exception No-throw guarantee.
   */
  constexpr auto is_open() const noexcept { return m_b_open; }

  /**
   * @brief Returns true if tile is a mine.
   * @return True if Tile is mine, false if not.
   * @pre Return value is undefined if value is yet to be assigned.
   * @post -
   * @exception No-throw guarantee.
   */
  constexpr bool is_mine() const noexcept { return m_tile_value == TILE_MINE; }

  /**
   * @brief Returns true if tile has a value of empty.
   * @return True if empty, false otherwise.
   * @pre Return value is undefined if value is yet to be assigned.
   * @post -
   * @exception No-throw guarantee.
   */
  constexpr bool is_empty() const noexcept {
    return m_tile_value == TILE_EMPTY;
  }

  /**
   * @brief Returns true if value is a number, e.g. not empty nor mine.
   * @return True if is number, false otherwise.
   * @pre Return value is undefined if value is yet to be assigned.
   * @post -
   * @exception No-throw guarantee.
   */
  constexpr bool is_number() const noexcept {
    return TILE_EMPTY < m_tile_value && m_tile_value < TILE_MINE;
  }

  /**
   * @brief Set value to mine.
   * @return void
   * @pre -
   * @post Tile's value is equal to TILE_MINE.
   * @exception No-throw guarantee.
   */
  constexpr void set_mine() noexcept { m_tile_value = TILE_MINE; }

  /**
   * @brief Set value to empty.
   * @return void
   * @pre -
   * @post Tile's value is equal to TILE_EMPTY.
   * @exception No-throw guarantee.
   */
  constexpr void set_empty() noexcept { m_tile_value = TILE_EMPTY; }

  /**
   * @brief Set open if Tile isn't flagged.
   * @return void
   * @pre Behaviour is undefined if flag state is yet to be assigned a value.
   * @post Tile is open if it isn't flagged. Tile stays unmodified if Tile is
   * flagged.
   * @exception No-throw guarantee.
   */
  constexpr void set_open() noexcept {
    if (!m_b_flagged) m_b_open = true;
  }
  // @brief Opens tile. Tile is set open whether or not tile is flagged.
  // Implemented for effiency purposes.
  constexpr void set_open_unguarded() noexcept { m_b_open = true; }
  // @brief Closes tile.
  constexpr void set_closed() noexcept { m_b_open = false; }
  // @brief Sets tile flagged if tile is not open.
  constexpr void set_flagged() noexcept {
    if (!m_b_open) m_b_flagged = true;
  }
  // @brief Sets tile flagged. Tile is set flagged whether or not tile is
  // open. Implemented for effiency purposes.
  constexpr void set_flagged_unguarded() noexcept { m_b_flagged = true; }
  // @brief Sets tile unflagged.
  constexpr void set_unflagged() noexcept { m_b_flagged = false; }

  // @brief Toggles whether tile is flagged or not. Flagging is conditional in
  // the sense that opened tiles won't be flagged.
  constexpr void toggle_flag() noexcept {
    if (m_b_flagged)
      set_unflagged();
    else
      set_flagged();
  }

  // @brief Promotes tile to a higher value. Used when determining values for
  // non-mine positions on the board. Doesn't promote tiles valued 8 or mines.
  constexpr void promote() noexcept {
    if (m_tile_value < TILE_8) ++m_tile_value;
  }

  // @brief Clears %BoardTile to have value of closed, unflagged and empty tile.
  constexpr void clear() noexcept {
    reset();
    set_empty();
  }

  // @brief Resets %BoardTile to have value of closed and unflagged. Tile's
  // board value is left untouched.
  constexpr void reset() noexcept {
    set_closed();
    set_unflagged();
  }
};

}  // namespace rake

#endif