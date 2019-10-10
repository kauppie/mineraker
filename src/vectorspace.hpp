#ifndef VECTORSPACE_HPP
#define VECTORSPACE_HPP

#include <exception>
#include <memory>
#include <vector>

#include "mineraker.hpp"

namespace rake {

// VectorSpace declaration.
template <typename T> class VectorSpace;

/**
 * TODO: Informational comment.
 */
template <typename T> class VectorSpace {
  using vec_type = std::vector<T>;
  using pointer_type = std::shared_ptr<vec_type>;

public:
  explicit VectorSpace() : m_vec_list(), m_reserve_size(0) {}
  VectorSpace(size_type start_size, size_type reserve_size)
      : m_reserve_size(reserve_size) {
    space_size(start_size);
  }
  ~VectorSpace() {}

  void vectors_reserve(size_type reserve) {
    for (auto &vec_p : m_vec_list)
      if (vec_p)
        vec_p->reserve(reserve);
    m_reserve_size = reserve;
  }

  auto vectors_reserve() const { return m_reserve_size; }

  void space_size(size_type vec_num) {
    m_vec_list.resize(vec_num);
    for (auto &vec_p : m_vec_list)
      vec_p = std::make_shared<vec_type>();
    if (m_reserve_size > 0)
      vectors_reserve(m_reserve_size);
  }

  auto space_size() const { return m_vec_list.size(); }

  pointer_type acquire() {
    for (auto &vec_p : m_vec_list) {
      if (vec_p.use_count() == 1) {
        vec_p->resize(0);
        return vec_p;
      }
    }
    auto back = m_vec_list.emplace_back(std::make_shared<vec_type>());
    back->reserve(m_reserve_size);
    return back;
  }

private:
  std::vector<std::shared_ptr<vec_type>> m_vec_list;
  size_type m_reserve_size;
};
} // namespace rake

#endif