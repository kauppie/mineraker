#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

template<typename T> void print_vec(const vector<T>& vec) {
  for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
    if (iter == vec.begin()) {
      cout << *iter;
    } else
      cout << ' ' << *iter;
  }
  cout << endl;
}

int main() {
  vector<int> v1 = {1, 2, 3};
  vector<int> v2 = {3, 4, 5};
  vector<int> v3;
  vector<int> v4;

  print_vec(v1);
  print_vec(v2);
  print_vec(v3);
  print_vec(v4);

  std::set_union(v1.begin(), v1.end(), v2.begin(), v2.end(),
                 std::back_inserter(v3));
  print_vec(v3);
  set_difference(v3.begin(), v3.end(), v2.begin(), v2.end(), back_inserter(v4));

  print_vec(v4);

  return EXIT_SUCCESS;
}