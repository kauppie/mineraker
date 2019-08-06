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
 * Links game states, %MineBoard object and window rendering in a single class
 * with simple to call methods.
 */
class GameManager {
public:
  GameManager(WindowManager *window, MineBoard *board)
      : m_window(window), m_board(board) {}
  ~GameManager() {}

  void open_from(int mouse_x, int mouse_y) {
    // m_board->open_from_tile((my / tiley) * mb.width() + (mx / tilex));
  }
  void flag_from(size_type n_tile) { m_board->flag_from_tile(n_tile); }
  void render() {
    if (m_window == nullptr || m_board == nullptr)
      throw "Error: window and/or board objects unassigned to gamemanager.";
    auto width = m_window->width(), height = m_window->height();
    auto b_width = m_board->width(), b_height = m_board->height();

    b_width = std::max(b_width, static_cast<size_type>(1));
    auto tile_width = width / b_width, tile_height = height / b_height;

    // Change to texture object inside GameManager class.
    Texture texture(m_window->renderer(), "img/medium.png");
    auto clip_width = texture.width() / 4, clip_height = texture.height() / 3;

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

      texture.render(m_window->renderer(), &clip, &dst_rect);
    }
  }

private:
  WindowManager *m_window;
  MineBoard *m_board;
  Texture m_tiles_texture;
};

} // namespace rake

#endif