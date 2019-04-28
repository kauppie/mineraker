#ifndef MINEBOARDFORMAT_HPP
#define MINEBOARDFORMAT_HPP

#include <sstream>
#include <string>
#include <string_view>

#include "mineboard.hpp"

namespace rake {

class MineBoardFormat {
public:
  MineBoardFormat() {}
  ~MineBoardFormat() {}

  static std::string format(const MineBoard &sb) {
    std::stringstream ss;
    ss << "[" << sb.mine_count() << "][" << sb.seed() << "];";
    return ss.str();
  }
  static MineBoard parse(std::string_view view) { return MineBoard{}; }
};

} // namespace rake

#endif