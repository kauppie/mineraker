#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

template<typename T, typename Pred>
void print_if(const std::vector<T>& vec, Pred pred) {
  for (auto& v : vec) {
    if (pred(v)) {
      std::cout << v << std::endl;
    }
  }
}

bool under_age(int age) { return age < 18; }

int main() {
  std::vector<int> vec = {5, 8, 3, 23, 3, 91, 182, 4};

  print_if(vec, &under_age);
}