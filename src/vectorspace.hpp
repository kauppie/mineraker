#ifndef VECTORSPACE_HPP
#define VECTORSPACE_HPP

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
  using reference = vec_type &;
  using const_reference = const vec_type &;
  using pointer_type = std::shared_ptr<vec_type>;

public:
  explicit VecSpaceInstance(const pointer_type &vec) { m_vector = vec; }
  explicit VectorSpaceInstance() = delete;
  ~VecSpaceInstance() {}

  T &operator[](size_type idx) { return m_vector->operator[](idx); }

  // @brief Get function. It's undefined to call returned vector's destructor.
  reference get() { return *m_vector.get(); }

  // @brief Constant get function.
  const_reference get() const { return *m_vector.get(); }

  void emplace(const T &other) { m_vector->emplace_back(other); }

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
  explicit VectorSpace() : m_reserve_size(0) {}
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

  auto vectors_reserve() const { return m_vec_list.front()->capacity(); }

  void space_size(size_type vec_num) {
    m_vec_list.resize(vec_num);
    vectors_reserve(m_reserve_size);
  }

  auto space_size() const { return m_vec_list.size(); }

  instance_type acquire() {
    for (auto &vec_p : m_vec_list) {
      // std::shared_ptr.unique() is going to deprecate from C++20.
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