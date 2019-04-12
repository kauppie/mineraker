#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "sweepboard.h"

int main() {
  msgn::SweepBoard sb(71, 49, .1, time(0));

  auto t1 = std::chrono::high_resolution_clock::now();
  auto vec = sb.m_empty_tiles_empty_area(439);
  sb.m_open_neighbours(vec);
  auto t2 = std::chrono::high_resolution_clock::now();

  auto time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
  return 0;
}