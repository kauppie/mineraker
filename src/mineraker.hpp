#ifndef MINERAKER_HPP
#define MINERAKER_HPP

#include <cstddef>
#include <exception>
#include <string>

namespace rake {
using size_type = std::size_t;
using diff_type = std::ptrdiff_t;

class RException : public std::exception {
public:
  explicit RException(const char *what) : m_what(what) {}
  explicit RException(const std::string &what) : m_what(what) {}
  virtual ~RException() throw() {}
  virtual const char *what() const throw() { return m_what.c_str(); }

protected:
  std::string m_what;
};

} // namespace rake

#endif