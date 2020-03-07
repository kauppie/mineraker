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

  SolverIterator() {}
  ~SolverIterator() {}

  bool next_step() { return next_flag_step() || next_open_step(); }

  bool next_flag_step() {
    bool next_change = false;
    if (auto b_ptr = board_ptr.lock()) {

      for (auto tile_it = b_ptr->begin(); tile_it != b_ptr->end(); ++tile_it) {
        if (tile_it->is_open() && tile_it->is_number()) {
          auto neighbours = b_ptr->tile_neighbours(
              std::distance(b_ptr->begin(), tile_it),
              [&b_ptr](const pos_type& p) {
                return b_ptr->is_inside_bounds(p) && !b_ptr->at(p).is_flagged();
              });

          if (tile_it->value() ==
              std::count_if(neighbours.begin(), neighbours.end(),
                            [&b_ptr](const pos_type& p) {
                              return !b_ptr->at(p).is_open();
                            })) {
            std::for_each(
                neighbours.begin(), neighbours.end(),
                [&b_ptr](const pos_type& p) { b_ptr->at(p).set_flagged(); });
            next_change = true;
          } else {
            for (auto neigh_pos_it = neighbours.begin();
                 neigh_pos_it != neighbours.end(); ++neigh_pos_it) {
              auto tile_flag_value =
                  tile_it->value() -
                  std::count_if(neighbours.begin(), neighbours.end(),
                                [&b_ptr](const pos_type& p) {
                                  return b_ptr->at(p).is_flagged();
                                });
              auto neigh_neighbours = b_ptr->tile_neighbours(*neigh_pos_it);
              auto neighbour_flag_value =
                  b_ptr->at(*neigh_pos_it).value() -
                  std::count_if(neigh_neighbours.begin(),
                                neigh_neighbours.end(),
                                [&b_ptr](const pos_type& p) {
                                  return b_ptr->at(p).is_flagged();
                                });

              if (b_ptr->at(*neigh_pos_it).is_open() &&
                  b_ptr->at(*neigh_pos_it).is_number() &&
                  tile_flag_value > neighbour_flag_value) {
                std::vector<pos_type> diff;
                std::set_difference(neighbours.begin(), neighbours.end(),
                                    neigh_neighbours.begin(),
                                    neigh_neighbours.end(),
                                    std::back_inserter(diff));
                auto diff_mines = tile_flag_value - neighbour_flag_value;

                if (diff.size() == diff_mines) {
                  std::copy(diff.begin(), diff.end(),
                            std::back_inserter(next_flag));
                  next_change = true;
                }
              }
            }
          }
        }
      }

    } else {
      throw std::bad_weak_ptr();
    }
    return next_change;
  }

  bool next_open_step() {
    bool next_change = false;
    if (auto b_ptr = board_ptr.lock()) {
      for (auto tile_it = b_ptr->begin(); tile_it != b_ptr->end(); ++tile_it) {
        if (tile_it->is_open() && tile_it->is_number()) {
          auto neighbours = b_ptr->tile_neighbours(
              std::distance(b_ptr->begin(), tile_it),
              [&b_ptr](const pos_type& p) {
                return b_ptr->is_inside_bounds(p) && !b_ptr->at(p).is_flagged();
              });

          if (tile_it->value() ==
              std::count_if(neighbours.begin(), neighbours.end(),
                            [&b_ptr](const pos_type& p) {
                              return b_ptr->at(p).is_flagged();
                            })) {
            std::for_each(
                neighbours.begin(), neighbours.end(),
                [&b_ptr](const pos_type& p) { b_ptr->at(p).set_open(); });
            next_change = true;
          } else {
            for (auto neigh_pos_it = neighbours.begin();
                 neigh_pos_it != neighbours.end(); ++neigh_pos_it) {
              auto tile_flag_value =
                  tile_it->value() -
                  std::count_if(neighbours.begin(), neighbours.end(),
                                [&b_ptr](const pos_type& p) {
                                  return b_ptr->at(p).is_flagged();
                                });

              auto neigh_neighbours = b_ptr->tile_neighbours(*neigh_pos_it);
              auto neighbour_flag_value =
                  b_ptr->at(*neigh_pos_it).value() -
                  std::count_if(neigh_neighbours.begin(),
                                neigh_neighbours.end(),
                                [&b_ptr](const pos_type& p) {
                                  return b_ptr->at(p).is_flagged();
                                });
              if (b_ptr->at(*neigh_pos_it).is_open() &&
                  b_ptr->at(*neigh_pos_it).is_number() &&
                  tile_flag_value == neighbour_flag_value &&
                  neighbours.size() > neigh_neighbours.size() &&
                  std::includes(neighbours.begin(), neighbours.end(),
                                neigh_neighbours.begin(),
                                neigh_neighbours.end())) {
                std::set_difference(neighbours.begin(), neighbours.end(),
                                    neigh_neighbours.begin(),
                                    neigh_neighbours.end(),
                                    std::back_inserter(next_open));
                next_change = true;
              }
            }
          }
        }
      }
    } else {
      throw std::bad_weak_ptr();
    }
    return next_change;
  }

  void reset() noexcept {
    next_open.clear();
    next_flag.clear();
    board_ptr.reset();
  }

private:
  std::vector<pos_type> next_open;
  std::vector<pos_type> next_flag;
  std::weak_ptr<Mineboardbase> board_ptr;
};

} // namespace rake

#endif
