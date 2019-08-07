#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include <algorithm>
#include <exception>
#include <memory>

#include "SDL2/SDL.h"

#include "boardtile.hpp"
#include "mineboard.hpp"
#include "mineraker.hpp"
#include "texture.hpp"
#include "windowmanager.hpp"

namespace rake {

/**
 * Literally manages game from windows to board events. Does texture association
 * and clipping.
 */
class GameManager {
public:
  explicit GameManager()
      : m_window(nullptr), m_board(nullptr), m_tile_texture(nullptr) {}
  explicit GameManager(WindowManager *windowmanager, MineBoard *mineboard,
                       Texture *tile_texture) {
    init(windowmanager, mineboard, tile_texture);
  }
  ~GameManager() {}

  void init(WindowManager *windowmanager, MineBoard *mineboard,
            Texture *tile_texture) {
    m_window = windowmanager;
    m_board = mineboard;
    m_tile_texture = tile_texture;

    auto clip_width = m_tile_texture->width() / TEXTURE_WIDTH_COUNT,
         clip_height = m_tile_texture->height() / TEXTURE_HEIGHT_COUNT;

    m_tiles_from_texture = {
        SDL_Rect{clip_width, 0, clip_width, clip_height},
        {0, clip_height, clip_width, clip_height},
        {clip_width, clip_height, clip_width, clip_height},
        {2 * clip_width, clip_height, clip_width, clip_height},
        {3 * clip_width, clip_height, clip_width, clip_height},
        {0, 2 * clip_height, clip_width, clip_height},
        {clip_width, 2 * clip_height, clip_width, clip_height},
        {2 * clip_width, 2 * clip_height, clip_width, clip_height},
        {3 * clip_width, 2 * clip_height, clip_width, clip_height},
        {0, 0, clip_width, clip_height},
        {2 * clip_width, 0, clip_width, clip_height},
        {3 * clip_width, 0, clip_width, clip_height}};
  }

  // Opens specified tile from mouse coordinates.
  void open_from(int mouse_x, int mouse_y) {
    m_board->open_tile(m_mouse_to_index(mouse_x, mouse_y));
  }

  // Flags specified tile from mouse coordinates.
  void flag_from(int mouse_x, int mouse_y) {
    m_board->flag_tile(m_mouse_to_index(mouse_x, mouse_y));
  }

  // Renders the board to the window.
  void render() {
    if (m_window == nullptr || m_board == nullptr ||
        m_tile_texture == nullptr) {
      std::cerr << "\nError: Incomplete Gamemanager.";
      return;
    }
    auto tile_edge = std::min(m_window->width() / m_board->width(),
                              m_window->height() / m_board->height());
    SDL_Rect dst_rect{0, 0, tile_edge, tile_edge};

    for (size_type i = 0; i < m_board->tile_count(); ++i) {
      auto tile = m_board->m_tiles[i];
      SDL_Rect clip;
      if (tile.is_open())
        clip = m_tiles_from_texture[tile.value()];
      else if (tile.is_flagged())
        clip = m_tiles_from_texture[11];
      else
        clip = m_tiles_from_texture[10];

      dst_rect.x = i % m_board->width() * tile_edge +
                   (m_window->width() - tile_edge * m_board->width()) / 2;
      dst_rect.y = i / m_board->width() * tile_edge +
                   (m_window->height() - tile_edge * m_board->height()) / 2;

      m_tile_texture->render(*m_window, &clip, &dst_rect);
    }
  }

private:
  size_type m_mouse_to_index(int mouse_x, int mouse_y) {
    auto tile_edge = std::min(m_window->width() / m_board->width(),
                              m_window->height() / m_board->height());
    auto x_offset = (m_window->width() - tile_edge * m_board->width()) / 2;
    auto y_offset = (m_window->height() - tile_edge * m_board->height()) / 2;
    auto x = (mouse_x - x_offset) / tile_edge;
    auto y = (mouse_y - y_offset) / tile_edge;
    // If %x is out of bounds, set result out of bounds.
    if (mouse_x >= m_window->width() - x_offset)
      return m_board->width() * m_board->height();
    return y * m_board->width() + x;
  }

  WindowManager *m_window;
  MineBoard *m_board;
  Texture *m_tile_texture;

  // Array to store texture clipping coordinates.
  std::array<SDL_Rect, TEXTURE_WIDTH_COUNT * TEXTURE_HEIGHT_COUNT>
      m_tiles_from_texture;
};

} // namespace rake

#endif