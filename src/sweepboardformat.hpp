#ifndef SWEEPBOARDFORMAT_HPP
#define SWEEPBOARDFORMAT_HPP

#include <sstream>
#include <string>
#include <string_view>

#include "sweepboard.hpp"

namespace msgn {

class SweepBoardFormat {
public:
  SweepBoardFormat() {}
  ~SweepBoardFormat() {}

  static std::string to_string(const SweepBoard &sb) {
    std::stringstream ss;
    ss << "[" << sb.mine_count() << "][" << sb.seed() << "];";
    return ss.str();
  }
  static SweepBoard from_string(std::string_view view) { return SweepBoard{}; }
};

} // namespace msgn

#endif