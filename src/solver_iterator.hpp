#ifndef RAKE_SOLVER_ITERATOR_HPP
#define RAKE_SOLVER_ITERATOR_HPP

#include <algorithm>
#include <memory>
#include <vector>

#include "mineboardbase.hpp"

namespace rake {

class SolverIterator {
public:
  using size_type = std::size_t;
  using diff_type = std::ptrdiff_t;

  using pos_type = Position_t;

  struct Minearea {
    std::vector<pos_type> tiles;
    size_type mines;
  };

  SolverIterator(std::shared_ptr<Mineboardbase> ptr) : board_ptr(ptr) {}
  ~SolverIterator() {}

  void next_step() {
    next_flag_step();
    next_open_step();
  }

  void next_flag_step() {
    for (auto tile_it = board_ptr->begin(); tile_it != board_ptr->end();
         ++tile_it) {
      if (tile_it->is_open() && tile_it->is_number()) {

        auto all_neighbours = board_ptr->tile_neighbours(
            std::distance(board_ptr->begin(), tile_it));

        if (tile_it->value() ==
            std::count_if(all_neighbours.begin(), all_neighbours.end(),
                          [this](const pos_type& p) {
                            return !board_ptr->at(p).is_open();
                          })) {
          std::copy(all_neighbours.begin(), all_neighbours.end(),
                    std::back_inserter(next_flag));
        } else {
          for (auto neigh_pos_it = all_neighbours.begin();
               neigh_pos_it != all_neighbours.end(); ++neigh_pos_it) {
            // Alias for neighbouring %Tile object.
            auto& neigbour = board_ptr->at(*neigh_pos_it);

            if (neigbour.is_open() && neigbour.is_number()) {

              auto tile_flag_value =
                  tile_it->value() -
                  std::count_if(all_neighbours.begin(), all_neighbours.end(),
                                [this](const pos_type& p) {
                                  return board_ptr->at(p).is_flagged();
                                });
              auto all_neigh_neighbours =
                  board_ptr->tile_neighbours(*neigh_pos_it);
              auto neighbour_flag_value =
                  neigbour.value() -
                  std::count_if(all_neigh_neighbours.begin(),
                                all_neigh_neighbours.end(),
                                [this](const pos_type& p) {
                                  return board_ptr->at(p).is_flagged();
                                });

              if (tile_flag_value > neighbour_flag_value) {
                std::vector<pos_type> diff;
                // Not flagged nor opened neighbours.
                auto not_fl_op_neighbours = not_flagged_opened_neighbours(
                    std::distance(board_ptr->begin(), tile_it));
                // Not flagged nor opened neighbours of neighbour.
                auto neigh_not_fl_op_neighbours =
                    not_flagged_opened_neighbours(*neigh_pos_it);

                std::set_difference(
                    not_fl_op_neighbours.begin(), not_fl_op_neighbours.end(),
                    neigh_not_fl_op_neighbours.begin(),
                    neigh_not_fl_op_neighbours.end(), std::back_inserter(diff));
                auto diff_mines = tile_flag_value - neighbour_flag_value;

                if (diff.size() == diff_mines) {
                  std::copy(diff.begin(), diff.end(),
                            std::back_inserter(next_flag));
                } else if (diff.size() > diff_mines) {
                  mine_areas.emplace_back(Minearea{diff, diff_mines});
                }
              }
            }
          }
        }
      }
    }
  }

  void next_open_step() {
    for (auto tile_it = board_ptr->begin(); tile_it != board_ptr->end();
         ++tile_it) {
      if (tile_it->is_open() && tile_it->is_number()) {

        auto not_fl_op_neighbours = not_flagged_opened_neighbours(
            std::distance(board_ptr->begin(), tile_it));
        auto all_neighbours = board_ptr->tile_neighbours(
            std::distance(board_ptr->begin(), tile_it));

        auto tile_flag_value =
            tile_it->value() -
            std::count_if(all_neighbours.begin(), all_neighbours.end(),
                          [this](const pos_type& p) {
                            return board_ptr->at(p).is_flagged();
                          });

        if (tile_flag_value ==
            std::count_if(not_fl_op_neighbours.begin(),
                          not_fl_op_neighbours.end(),
                          [this](const pos_type& p) {
                            return board_ptr->at(p).is_flagged();
                          })) {
          std::copy(not_fl_op_neighbours.begin(), not_fl_op_neighbours.end(),
                    std::back_inserter(next_open));
        } else {
          for (auto neigh_pos_it = all_neighbours.begin();
               neigh_pos_it != all_neighbours.end(); ++neigh_pos_it) {

            if (board_ptr->at(*neigh_pos_it).is_open() &&
                board_ptr->at(*neigh_pos_it).is_number()) {

              auto neigh_not_fl_op_neighbours =
                  not_flagged_opened_neighbours(*neigh_pos_it);
              auto all_neigh_neighbours =
                  board_ptr->tile_neighbours(*neigh_pos_it);
              auto neighbour_flag_value =
                  board_ptr->at(*neigh_pos_it).value() -
                  std::count_if(all_neigh_neighbours.begin(),
                                all_neigh_neighbours.end(),
                                [this](const pos_type& p) {
                                  return board_ptr->at(p).is_flagged();
                                });
              if (tile_flag_value == neighbour_flag_value &&
                  not_fl_op_neighbours.size() >
                      neigh_not_fl_op_neighbours.size() &&
                  std::includes(not_fl_op_neighbours.begin(),
                                not_fl_op_neighbours.end(),
                                neigh_not_fl_op_neighbours.begin(),
                                neigh_not_fl_op_neighbours.end())) {

                std::set_difference(not_fl_op_neighbours.begin(),
                                    not_fl_op_neighbours.end(),
                                    neigh_not_fl_op_neighbours.begin(),
                                    neigh_not_fl_op_neighbours.end(),
                                    std::back_inserter(next_open));
              }
            }
          }
        }
      }
    }
  }

  void reset() noexcept {
    clear();
    board_ptr.reset();
  }

  bool open_and_flag() {
    if (next_flag.size() == 0 && next_open.size() == 0)
      return false;

    for (auto pos : next_flag) {
      board_ptr->at(pos).set_flagged();
    }
    for (auto pos : next_open) {
      board_ptr->open(pos);
    }
    clear();
    return true;
  }

private:
  void clear() noexcept {
    next_open.clear();
    next_flag.clear();
    mine_areas.clear();
  }

  std::vector<pos_type> not_flagged_opened_neighbours(pos_type pos) const {
    return board_ptr->tile_neighbours(pos, [this](const pos_type& p) {
      return !board_ptr->at(p).is_flagged() && !board_ptr->at(p).is_open();
    });
  }

  std::vector<pos_type> not_flagged_opened_neighbours(size_type idx) const {
    return not_flagged_opened_neighbours(board_ptr->idx_to_pos(idx));
  }

  std::vector<pos_type> next_open;
  std::vector<pos_type> next_flag;
  std::vector<Minearea> mine_areas;

  std::shared_ptr<Mineboardbase> board_ptr;
};

} // namespace rake

#endif
