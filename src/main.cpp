#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "sweepboard.h"

int main() {
  auto t1 = std::chrono::high_resolution_clock::now();
  SweepBoard sb(16, 16, .4);
  auto t2 = std::chrono::high_resolution_clock::now();

  std::cout
      << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()
      << " ns\n";
  return 0;
}