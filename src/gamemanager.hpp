#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include <algorithm>
#include <exception>
#include <memory>

#include "SDL2/SDL.h"

#include "boardtile.hpp"
#include "mineboard.hpp"
#include "mineboardsolver.hpp"
#include "mineraker.hpp"
#include "text.hpp"
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
  explicit GameManager(WindowManager* windowmanager, MineBoard* mineboard,
                       Texture* tile_texture) {
    init(windowmanager, mineboard, tile_texture);
  }
  ~GameManager() {}

  void init(WindowManager* windowmanager, MineBoard* mineboard,
            Texture* tile_texture) {
    m_window = windowmanager;
    m_board = mineboard;
    m_tile_texture = tile_texture;

    auto clip_width = m_tile_texture->width() / TEXTURE_WIDTH_COUNT,
         clip_height = m_tile_texture->height() / TEXTURE_HEIGHT_COUNT;

    m_tiles_from_texture = {
        SDL_Rect{clip_width, 0, clip_width, clip_height},
        SDL_Rect{0, clip_height, clip_width, clip_height},
        SDL_Rect{clip_width, clip_height, clip_width, clip_height},
        SDL_Rect{2 * clip_width, clip_height, clip_width, clip_height},
        SDL_Rect{3 * clip_width, clip_height, clip_width, clip_height},
        SDL_Rect{0, 2 * clip_height, clip_width, clip_height},
        SDL_Rect{clip_width, 2 * clip_height, clip_width, clip_height},
        SDL_Rect{2 * clip_width, 2 * clip_height, clip_width, clip_height},
        SDL_Rect{3 * clip_width, 2 * clip_height, clip_width, clip_height},
        SDL_Rect{0, 0, clip_width, clip_height},
        SDL_Rect{2 * clip_width, 0, clip_width, clip_height},
        SDL_Rect{3 * clip_width, 0, clip_width, clip_height}};
  }

  // Opens specified tile from mouse coordinates.
  void open_from(int mouse_x, int mouse_y) {
    size_type idx = m_mouse_to_index(mouse_x, mouse_y);
    bool was_first = m_board->state() == rake::MineBoard::State::FIRST_MOVE;
    m_board->open_tile(idx);
    if (was_first)
      find_solvable_game(idx);
  }

  // Flags specified tile from mouse coordinates.
  void flag_from(int mouse_x, int mouse_y) {
    m_board->flag_tile(m_mouse_to_index(mouse_x, mouse_y));
  }

  // Open all tiles which can be determined by their neighbouring flagged tiles
  // and their own value.
  void open_by_flagged() {
    MineBoardSolver mbs(*m_board);
    while (mbs.open_by_flagged())
      ;
  }

  void find_solvable_game(size_type idx) {
    MineBoardSolver mbs(*m_board);
    size_type i = 0;
    while (m_board->state() != rake::MineBoard::State::GAME_WIN) {
      auto seed =
          std::chrono::high_resolution_clock::now().time_since_epoch().count();
      m_board->init(m_board->width(), m_board->height(), seed,
                    m_board->mine_count());
      m_board->open_tile(idx);
      if (mbs.b_solve())
        mbs.open_by_flagged();
      ++i;
    }
    m_board->init(m_board->width(), m_board->height(), m_board->seed(),
                  m_board->mine_count());
    m_board->open_tile(idx);
    std::cerr << "\niterations to find solvable: " << i;
  }

  // Renders the board to the window.
  void render() const {
    if (m_window == nullptr || m_board == nullptr ||
        m_tile_texture == nullptr) {
      std::cerr << "\nError: Incomplete Gamemanager.";
      return;
    }
    for (size_type i = 0; i < m_board->tile_count(); ++i) {
      auto clip = texture_clip_tile(m_board->m_tiles[i]);
      auto dst_rect = tile_dest(i);

      m_tile_texture->render(m_window->renderer(), &clip, &dst_rect);
    }
  }

private:
  SDL_Rect texture_clip_tile(const BoardTile& tile) const {
    SDL_Rect clip;
    if (tile.is_open())
      clip = m_tiles_from_texture[tile.value()];
    else if (tile.is_flagged())
      clip = m_tiles_from_texture[11];
    else
      clip = m_tiles_from_texture[10];
    return clip;
  }

  size_type m_mouse_to_index(int mouse_x, int mouse_y) const {
    auto tile_edge = tiles_edge();
    auto x_offset = x_tile_offset();
    // If %x is out of bounds, return result out of bounds.
    if (mouse_x >= m_window->width() - static_cast<int>(x_offset))
      return m_board->width() * m_board->height();
    auto x = (mouse_x - x_offset) / tile_edge;
    auto y = (mouse_y - y_tile_offset()) / tile_edge;
    return y * m_board->width() + x;
  }

  size_type x_tile_offset() const {
    return (m_window->width() - tiles_edge() * m_board->width()) / 2;
  }

  size_type y_tile_offset() const {
    return (m_window->height() - tiles_edge() * m_board->height()) / 2;
  }

  size_type tiles_edge() const {
    return std::min(m_window->width() / m_board->width(),
                    m_window->height() / m_board->height());
  }

  SDL_Rect tile_dest(size_type idx) const {
    auto bw = m_board->width();
    auto edge = tiles_edge();
    return {static_cast<int>(idx % bw * edge + x_tile_offset()),
            static_cast<int>(idx / bw * edge + y_tile_offset()),
            static_cast<int>(edge), static_cast<int>(edge)};
  }

  WindowManager* m_window;
  MineBoard* m_board;
  Texture* m_tile_texture;

  // Array to store texture clipping coordinates.
  std::array<SDL_Rect, TEXTURE_WIDTH_COUNT * TEXTURE_HEIGHT_COUNT>
      m_tiles_from_texture;
};

} // namespace rake

#endif