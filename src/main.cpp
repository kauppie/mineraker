#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "sweepboard.h"

int main() {
  msgn::SweepBoard sb(71, 49, .05, time(0));

  auto t1 = std::chrono::high_resolution_clock::now();
  sb.m_open_neighbours(sb.m_empty_tiles_empty_area(439));
  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout
      << "Old: "
      << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()
      << " ns\n";

  t1 = std::chrono::high_resolution_clock::now();
  sb.m_open_neighbours(sb.m_empty_tiles_empty_area_test(439));
  t2 = std::chrono::high_resolution_clock::now();

  std::cout
      << "New: "
      << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()
      << " ns\n";

  t1 = std::chrono::high_resolution_clock::now();
  sb.m_flood_queue(439);
  t2 = std::chrono::high_resolution_clock::now();

  std::cout
      << "flood_queue: "
      << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()
      << " ns\n";

  t1 = std::chrono::high_resolution_clock::now();
  sb.m_flood_stack(439);
  t2 = std::chrono::high_resolution_clock::now();

  std::cout
      << "flood_stack: "
      << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()
      << " ns\n";
  return 0;
}