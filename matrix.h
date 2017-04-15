#ifndef CGRAPH_MATRIX_H
#define CGRAPH_MATRIX_H

#include <array>
#include <cstdint>
#include <iostream>
#include <random>
#include <type_traits>
#include <vector>

namespace cgraph {

namespace {

template<typename T, typename R, typename M>
using Convertible = std::enable_if_t<std::is_convertible<T, R>::value, M>;

template<typename T, typename R, typename M>
using NotConvertible = std::enable_if_t<!std::is_convertible<T, R>::value, M>;

template<typename RNG>
static RNG &static_rng() {
  static RNG rng;
  return rng;
}

} // namespace

// Defines a matrix of Ts with m rows and n columns.
template<typename T, std::size_t m, std::size_t n>
class Matrix {
public:
  // Alias for the matrix's type.
  using ValueType = std::enable_if_t<std::is_arithmetic<T>::value, T>;

  // Constructs a matrix of the given size, with zeroed elements.
  Matrix() : data(m * n) { };

  // Constructs a matrix of the given size, with the given elements.
  template<typename... Args>
  Matrix(Args... args) : data({args...}) {
    static_assert(sizeof...(args) == m * n, "Wrong number of elements");
  }

  // Convert between different types.
  template<typename R>
  explicit Matrix(const Matrix<R, m, n> &mat) : Matrix() {
    for (std::size_t i = 0; i < m; i++) {
      for (std::size_t j = 0; j < n; j++) {
        (*this)(i, j) = static_cast<T>(mat(i,j));
      }
    }
  }

  // Returns an identity matrix.
  template<typename RT = Matrix<T, m, n>>
  static std::enable_if_t<m == n, RT> Identity() {
    RT mat;
    for (std::size_t i = 0; i < m; i++) {
      mat(i, i) = static_cast<T>(1);
    }
    return mat;
  }
  // Disable for non square matrices.
  template<typename RT = Matrix<T, m, n>>
  static std::enable_if_t<m != n, RT> Identity() = delete;

  // Generates a random matrix, defaulting to uniform between 0 and 1.
  template<
    typename Distribution = std::uniform_real_distribution<T>,
    typename RNG = std::default_random_engine
  >
  static Matrix<T, m, n> Random(
      Distribution &&dist = Distribution(static_cast<T>(0), static_cast<T>(1)),
      RNG &rng = static_rng<RNG>()) {
    Matrix<T, m, n> res;
    for (std::size_t i = 0; i < m; i++) {
      for (std::size_t j = 0; j < n; j++) {
        res(i, j) = dist(rng);
      }
    }
    return res;
  }

  // Generate a matrix filled with a given value.
  template<typename R, typename RT = Convertible<R, T, T>>
  static Matrix<T, m, n> Constant(R t) {
    Matrix<T, m, n> res;
    for (std::size_t i = 0; i < m; i++) {
      for (std::size_t j = 0; j < n; j++) {
        res(i, j) = static_cast<T>(t);
      }
    }
    return res;
  }
  // Remove for non convertible types.
  template<typename R, typename RT = NotConvertible<R, T, T>>
  static Matrix<RT, m, n> Constant(R t) = delete;

  // Generate a matrix filled with ones.
  static Matrix<T, m, n> Ones() {
    return Constant(1);
  }

  // Returns the total size of the matrix.
  std::size_t Size() { return m * n; }

  // Returns the number of rows in the matrix.
  std::size_t Rows() { return m; }

  // Returns the number of columns in the matrix.
  std::size_t Cols() { return n; }

  // Returns the element at i, j.
  T &operator()(std::size_t i, std::size_t j) {
    return data[i * n + j];
  }

  // Return a const reference to the element at i, j.
  const T &operator()(std::size_t i, std::size_t j) const {
    return data[i * n + j];
  }

  // Return a reference to the ith element of the vector.
  template<typename RT = T>
  std::enable_if_t<n == 1 || m == 1, RT> &operator()(std::size_t i) {
    return data[i];
  }
  // Return a const reference to the ith element of the vector.
  template<typename RT = T>
  const std::enable_if_t<n == 1 || m == 1, RT> &operator()(std::size_t i)
      const {
    return data[i];
  }
  // Delete for non-vectors.
  template<typename RT = T>
  std::enable_if_t<n != 1 && m != 1, RT> operator()(std::size_t i) const =
      delete;

  // Computes the matrix product of two matrices.
  template<typename R, std::size_t o>
  Matrix<std::common_type_t<R,T>, m, o> operator*(const Matrix<R, n, o> &mat)
      const {
    Matrix<std::common_type_t<T, R>, m, o> res;
    for (std::size_t k = 0; k < n; k++) {
      for (std::size_t j = 0; j < o; j++) {
        for (std::size_t i = 0; i < m; i++) {
          res(i, j) = static_cast<std::common_type_t<R,T>>(
            res(i, j) + (*this)(i, k) * mat(k, j)
          );
        }
      }
    }
    return res;
  }

  // Computes the sum of two matrices.
  template<typename R>
  Matrix<std::common_type_t<T, R>, m, n> operator+(const Matrix<R, m, n> &mat)
      const {
    auto res = static_cast<Matrix<std::common_type_t<T, R>, m, n>>(*this);
    for (std::size_t i = 0; i < m; i++) {
      for (std::size_t j = 0; j < n; j++) {
        res(i, j) = static_cast<std::common_type_t<R,T>>(
          res(i, j) + mat(i, j)
        );
      }
    }
    return res;
  }

  // Computes the difference of two matrices.
  template<typename R>
  Matrix<std::common_type_t<T, R>, m, n> operator-(const Matrix<R, m, n> &mat)
      const {
    auto res = static_cast<Matrix<std::common_type_t<T, R>, m, n>>(*this);
    for (std::size_t i = 0; i < m; i++) {
      for (std::size_t j = 0; j < n; j++) {
        res(i, j) = static_cast<std::common_type_t<R,T>>(
          res(i, j) - mat(i, j)
        );
      }
    }
    return res;
  }

  // "In place" product.
  template<typename R, std::size_t o>
  Matrix<Convertible<R, T, T>, m, o> &operator*=(const Matrix<R, n, o> &mat) {
    *this = *this * mat;
    return *this;
  }
  // Disable for differing types.
  template<typename R, std::size_t o>
  Matrix<NotConvertible<R, T, T>, m, o> &operator*=(
      const Matrix<R, n, o> &mat) = delete;

  // In place sum.
  template<typename R>
  Matrix<Convertible<R, T, T>, m, n> operator+=(const Matrix<R, m, n> &mat) {
    for (std::size_t i = 0; i < m; i++) {
      for (std::size_t j = 0; j < n; j++) {
        (*this)(i, j) = static_cast<T>((*this)(i, j) + mat(i, j));
      }
    }
    return *this;
  }
  // Disable for differing types.
  template<typename R, std::size_t o>
  Matrix<NotConvertible<R, T, T>, m, o> &operator+=(
      const Matrix<R, n, o> &mat) = delete;

  // In place difference.
  template<typename R>
  Matrix<Convertible<R, T, T>, m, n> operator-=(const Matrix<R, m, n> &mat) {
    for (std::size_t i = 0; i < m; i++) {
      for (std::size_t j = 0; j < n; j++) {
        (*this)(i, j) = static_cast<T>((*this)(i, j) - mat(i, j));
      }
    }
    return *this;
  }
  // Disable for differing types.
  template<typename R, std::size_t o>
  Matrix<NotConvertible<R, T, T>, m, o>
  &operator-=(const Matrix<R, n, o> &mat) = delete;

  // In place multiplication by a scalar.
  template<typename R>
  Matrix<Convertible<R, T, T>, m, n> operator*=(const R &val) {
    for (std::size_t i = 0; i < m; i++) {
      for (std::size_t j = 0; j < n; j++) {
        (*this)(i, j) = static_cast<T>((*this)(i, j) / val);
      }
    }
    return *this;
  }
  // Disable for differing types.
  template<typename R>
  Matrix<NotConvertible<R, T, T>, m, n> operator*=(const R &val) = delete;

  // In place division by a scalar.
  template<typename R>
  Matrix<Convertible<R, T, T>, m, n> operator/=(const R &val) {
    for (std::size_t i = 0; i < m; i++) {
      for (std::size_t j = 0; j < n; j++) {
        (*this)(i, j) = static_cast<T>((*this)(i, j) / val);
      }
    }
    return *this;
  }
  // Disable for differing types.
  template<typename R>
  Matrix<NotConvertible<R, T, T>, m, n> operator/=(const R &val) = delete;

  // In place addition by a scalar.
  template<typename R>
  Matrix<Convertible<R, T, T>, m, n> operator+=(const R &val) {
    for (std::size_t i = 0; i < m; i++) {
      for (std::size_t j = 0; j < n; j++) {
        (*this)(i, j) = static_cast<T>((*this)(i, j) + val);
      }
    }
    return *this;
  }
  // Disable for differing types.
  template<typename R>
  Matrix<NotConvertible<R, T, T>, m, n> operator+=(const R &val) = delete;

  // In place subtraction by a scalar.
  template<typename R>
  Matrix<Convertible<R, T, T>, m, n> operator-=(const R &val) {
    for (std::size_t i = 0; i < m; i++) {
      for (std::size_t j = 0; j < n; j++) {
        (*this)(i, j) = static_cast<T>((*this)(i, j) - val);
      }
    }
    return *this;
  }
  // Disable for differing types.
  template<typename R>
  Matrix<NotConvertible<R, T, T>, m, n> operator-=(const R &val) = delete;

  // 1x1 matrix can decay to a scalar.
  template<typename RT = T> operator std::enable_if_t<n * m == 1, RT>() {
    return data[0];
  }
  // Other matrices can't decay to a scalar.
  template<typename RT = T> operator std::enable_if_t<n * m != 1, RT>() =
      delete;

  // Transposes the given matrix.
  Matrix<T, m, n> Transpose() const {
    Matrix<T, m, n> mat;
    for (std::size_t i = 0; i < m; i++) {
      for (std::size_t j = 0; j < n; j++) {
        mat(j, i) = (*this)(i, j);
      }
    }
    return mat;
  }

private:
  std::vector<T> data;
};

// Write a matrix to an ostream.
template<typename T, std::size_t m, std::size_t n>
std::ostream &operator<<(std::ostream &os, const Matrix<T, m, n> &mat) {
  os << "[";
  for (std::size_t i = 0; i < m; i++) {
    os << (i==0 ? "" : " ") << "[ ";
    for (std::size_t j = 0; j < n; j++) {
      os << mat(i, j) << (j == n-1 ? "" : ",") << " ";
    }
    os << "]" << (i == m-1 ? "" : "\n");
  }
  os << "]\n";
  return os;
}

// Multiplication by a scalar.
template<typename T, std::size_t m, std::size_t n, typename R>
Matrix<std::common_type_t<R, T>, m, n> operator*(R val, Matrix<T, m, n> mat) {
  auto ret = static_cast<Matrix<std::common_type_t<R, T>, m, n>>(mat);
  for (std::size_t i = 0; i < n; i++) {
    for (std::size_t j = 0; j < n; j++) {
      ret(i, j) = static_cast<std::common_type_t<R, T>>(ret(i, j) * val);
    }
  }
  return ret;
}

// Other side.
template<typename T, std::size_t m, std::size_t n, typename R>
Matrix<std::common_type_t<R, T>, m, n> operator*(Matrix<T, m, n> mat, R val) {
  return val * mat;
}

// Division by a scalar.
template<typename T, std::size_t m, std::size_t n, typename R>
Matrix<std::common_type_t<R, T>, m, n> operator/(Matrix<T, m, n> mat, R val) {
  auto ret = static_cast<Matrix<std::common_type_t<R, T>, m, n>>(mat);
  for (std::size_t i = 0; i < n; i++) {
    for (std::size_t j = 0; j < n; j++) {
      ret(i, j) = static_cast<std::common_type_t<R, T>>(ret(i, j) / val);
    }
  }
  return ret;
}

// Declares a scalar type.
template<typename T>
using Scalar = T;

// Declares a row vector with n elements.
template<typename T, std::size_t n>
using RowVector = Matrix<T, 1, n>;

// Declares a column vector with n elements.
template<typename T, std::size_t n>
using ColVector = Matrix<T, n, 1>;

} // cgraph

#endif // CGRAPH_MATRIX_H
