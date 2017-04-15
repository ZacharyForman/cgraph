#ifndef CGRAPH_VARIABLE_H
#define CGRAPH_VARIABLE_H

#include <iostream>

namespace cgraph {

// Implementation of a general variable class.
template<typename T>
class Variable {
public:

  // Can construct a Variable from a piece of data.
  Variable(const T &t) : data(t) { }

  // Implicitly converts to a T.
  operator T() {
    return data;
  }
  operator T&() {
    return data;
  }
  operator const T&() const {
    return data;
  }

  // Access the data.
  T& operator()() {
    return data;
  }
  const T& operator()() const {
    return data;
  }

private:
  T data;
};

// Utility function to save typing.
template<typename T>
Variable<T> CreateVariable(const T &t) {
  return Variable<T>(t);
}

// Allow ostreaming of variables.
template<typename T>
std::ostream &operator<<(std::ostream &os, const Variable<T> &v) {
  os << static_cast<const T&>(v);
  return os;
}

// Alias for constants
template<typename T>
using Constant = const Variable<T>;

} // cgraph

#endif // CGRAPH_VARIABLE_H
