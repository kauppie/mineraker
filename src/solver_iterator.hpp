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
    if (auto b_ptr = board_ptr.lock()) {

      for (auto tile_it = b_ptr->begin(); tile_it != b_ptr->end(); ++tile_it) {
        if (tile_it->is_open() && tile_it->is_number()) {
        }
      }

    } else {
      throw std::bad_weak_ptr();
    }
  }

  bool next_open_step() {}

private:
  std::vector<pos_type> next_open;
  std::vector<pos_type> next_flag;
  std::weak_ptr<Mineboardbase> board_ptr;
};

} // namespace rake

#endif
