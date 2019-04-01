#include "sweepboard.h"

class SweepBoardController {
private:
  SweepBoard board;

public:
  SweepBoardController(std::size_t width_, std::size_t height_,
                       double mine_fill_percent,
                       uint32_t seed = std::mt19937_64::default_seed)
      : board(width_, height_, mine_fill_percent, seed) {}
  ~SweepBoardController() {}

  void foo(){
  }

private:
}; // end class SweepBoardController