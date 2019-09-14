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
template <typename T> class VecSpaceInstance {
  using vec_type = std::vector<T>;
  using pointer_type = std::shared_ptr<vec_type>;

public:
  explicit VecSpaceInstance(const pointer_type &vec) : m_vector(vec) {}
  ~VecSpaceInstance() { m_vector->resize(0); }

  // @brief Index operator to access element at given index.
  const T &operator[](size_type idx) const { return m_vector->operator[](idx); }

  // @brief Index operator to access element at given index.
  T &operator[](size_type idx) { return m_vector->operator[](idx); }

  // @brief Index operator to access element at given index. %idx is bounds
  // checked.
  const T &at(size_type idx) const {
    if (idx >= m_vector->size())
      throw std::out_of_range("Index is out of range.");
    return m_vector->operator[](idx);
  }

  // @brief Index operator to access element at given index. %idx is bounds
  // checked.
  T &at(size_type idx) {
    if (idx >= m_vector->size())
      throw std::out_of_range("Index is out of range.");
    return m_vector->operator[](idx);
  }

  // @brief Get function. It's undefined to call returned vector's destructor.
  vec_type &get() { return *m_vector.get(); }

  // @brief Constant get function.
  const vec_type &get() const { return *m_vector.get(); }

  auto front() const { return m_vector->front(); }
  auto begin() const { return m_vector->begin(); }
  auto end() const { return m_vector->end(); }

  void emplace_back(const T &other) { m_vector->emplace_back(other); }

  auto size() const { return m_vector->size(); }

private:
  pointer_type m_vector;
};

/**
 * TODO: Informational comment.
 */
template <typename T> class VectorSpace {
  using vec_type = std::vector<T>;
  using pointer_type = std::shared_ptr<vec_type>;
  using instance_type = VecSpaceInstance<T>;

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

  instance_type acquire() {
    for (auto &vec_p : m_vec_list) {
      if (vec_p.use_count() == 1)
        return instance_type(vec_p);
    }
    m_vec_list.emplace_back(std::make_shared<vec_type>());
    m_vec_list.back()->reserve(m_reserve_size);
    return instance_type(m_vec_list.back());
  }

private:
  std::vector<std::shared_ptr<vec_type>> m_vec_list;
  size_type m_reserve_size;
};
} // namespace rake

#endif