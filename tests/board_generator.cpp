#include <chrono>
#include <iostream>
#include <random>

#include "../src/mineboardbase.hpp"

using namespace std;
using namespace rake;
using namespace chrono;

void test() {
  using size_type = size_t;

  random_device rd;
  mt19937_64 gen(rd());

  size_type size = 64 * 32 * 4;
  size_type left_untouched = 9;
  size_type mines_to_spread = 2000;

  auto t1 = high_resolution_clock::now();
  {
    std::vector<size_type> mines_to_set(size - left_untouched);

    std::iota(mines_to_set.begin(), mines_to_set.end(), 0);
    std::shuffle(mines_to_set.begin(), mines_to_set.end(), gen);

    for (auto m = mines_to_set.begin();
         m != mines_to_set.begin() + mines_to_spread; ++m) {
    }
  }
  auto t2 = high_resolution_clock::now();
  {
    std::vector<size_type> mines_to_set(size - left_untouched);
    std::vector<bool> set_mine(size - left_untouched, false);
    for (size_type i = 0; i < mines_to_spread; ++i)
      set_mine[i] = true;
    std::shuffle(set_mine.begin(), set_mine.end(), gen);

    mines_to_set.reserve(set_mine.size());
    for (size_type i = 0; i < set_mine.size(); ++i) {
      if (set_mine[i])
        mines_to_set.emplace_back(i);
    }
  }
  auto t3 = high_resolution_clock::now();
  {
    std::uniform_int_distribution<size_type> dis(0, size - left_untouched - 1);
    std::vector<bool> at(size, false);

    for (size_type mines_spread = 0; mines_spread < mines_to_spread;) {
      auto idx = dis(gen);

      // This if statement should become unnecessary when mapping is used.
      // No IF statements at all. Pure fast, maybe?
      if (!at[idx]) {
        at[idx] = true;
        ++mines_spread;
      }
    }
  }
  auto t4 = high_resolution_clock::now();

  cout << "No if method: " << duration<double, micro>(t2 - t1).count()
       << " us\n"
       << "Many if method: " << duration<double, micro>(t3 - t2).count()
       << " us\n"
       << "Distribution method: " << duration<double, micro>(t4 - t3).count()
       << " us\n";
}

void print_board(const Mineboardbase& mb) {
  for (Position_t::value_type y = 0; y < mb.height(); ++y) {
    for (Position_t::value_type x = 0; x < mb.width(); ++x) {
      auto tile = mb.at({x, y});
      auto value = tile.value();

      if (tile.is_open()) {
        switch (value) {
        case BoardTile::TILE_EMPTY:
          cout << '_';
          break;
        case BoardTile::TILE_MINE:
          cout << '*';
          break;
        default:
          cout << (int)value;
          break;
        }
      } else {
        cout << 'X';
      }
    }

    cout << endl;
  }
}

int main() {
  Mineboardbase mb(8, 8);
  random_device rd;
  mt19937_64 gen(rd());

  size_t minecount;
  cout << "Minecount: ";
  cin >> minecount;

  mb.generate(minecount, {0, 0}, gen);

  while (true) {
    print_board(mb);
    int x, y;
    cin >> x >> y;

    mb.open({x, y});
  }
}