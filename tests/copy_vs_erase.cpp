#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

using namespace std;
using namespace chrono;

bool pred(int i) { return i < 3; }

int main() {
  constexpr size_t size = 8;

  //cout << "Size: " << size << endl;

  random_device rd;
  auto seed = rd();

  //cout << "Copy_if:" << endl;
  {
    mt19937_64 gen(seed);
    uniform_int_distribution<> dis(0, size - 1);

    auto t1 = high_resolution_clock::now();
    array<int, size> arr;

    for (auto& i : arr) {
      i = dis(gen);
    }

    std::vector<int> vec;
    vec.reserve(size);

    copy_if(arr.begin(), arr.end(), back_inserter(vec),
            [](int i) { return pred(i); });
    auto t2 = high_resolution_clock::now();

    //cout << duration<double, micro>(t2 - t1).count() << " us" << endl;
    //cout << "Size after transform: " << vec.size() << endl;
    //cout << "Max elem: " << *max_element(vec.begin(), vec.end()) << endl;
  }
  //cout << "Remove_if:" << endl;
  {
    mt19937_64 gen(seed);
    uniform_int_distribution<> dis(0, size - 1);

    auto t1 = high_resolution_clock::now();
    std::vector<int> vec;

    vec.resize(size);

    for (auto& i : vec) {
      i = dis(gen);
    }

    auto end =
        std::remove_if(vec.begin(), vec.end(), [](int i) { return !pred(i); });
    vec.resize(std::distance(vec.begin(), end));
    auto t2 = high_resolution_clock::now();
    //cout << duration<double, micro>(t2 - t1).count() << " us" << endl;
    //cout << "Size after transform: " << vec.size() << endl;
    //cout << "Max elem: " << *max_element(vec.begin(), end) << endl;
  }
}