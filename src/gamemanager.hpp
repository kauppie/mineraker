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
  GameManager() : m_window(nullptr), m_board(nullptr) {}
  ~GameManager() {}

  void init(WindowManager *windowmanager, MineBoard *mineboard,
            Texture *tile_texture) {
    m_window = windowmanager;
    m_board = mineboard;
    m_tiles_texture = tile_texture;
  }

  void open_from(int mouse_x, int mouse_y) {
    // m_board->open_tile((my / tiley) * mb.width() + (mx / tilex));
  }
  void flag_from(size_type n_tile) { m_board->flag_tile(n_tile); }
  void render() {
    if (m_window == nullptr || m_board == nullptr ||
        m_tiles_texture == nullptr) {
      std::cerr << "\nError: Incomplete Gamemanager.";
      return;
    }
    auto width = m_window->width(), height = m_window->height();
    auto b_width = m_board->width(), b_height = m_board->height();

    b_width = std::max(b_width, static_cast<size_type>(1));
    auto tile_width = width / b_width, tile_height = height / b_height;

    // Change to texture object inside GameManager class.
    Texture texture(m_window->renderer(), "img/medium.png");
    auto clip_width = m_tiles_texture->width() / 4,
         clip_height = m_tiles_texture->height() / 3;

    std::array<SDL_Rect, 12ul> tiles_from_texture = {
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
    SDL_Rect dst_rect{0, 0, tile_width, tile_height};

    for (size_type i = 0; i < m_board->tile_count(); ++i) {
      auto tile = m_board->m_tiles[i];
      SDL_Rect clip;
      if (tile.is_open())
        clip = tiles_from_texture[tile.value()];
      else if (tile.is_flagged())
        clip = tiles_from_texture[11];
      else /* if (!tile.is_open()) */
        clip = tiles_from_texture[10];

      dst_rect.x = i % b_width * tile_width;
      dst_rect.y = i / b_width * tile_height;

      m_tiles_texture->render(m_window->renderer(), &clip, &dst_rect);
    }
  }

private:
  WindowManager *m_window;
  MineBoard *m_board;
  Texture *m_tiles_texture;
};

} // namespace rake

#endif