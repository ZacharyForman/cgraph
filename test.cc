#include "expression.h"
#include "matrix.h"
#include "variable.h"

#include <cstddef>
#include <iostream>

int main() {
  using namespace cgraph;

  Matrix<float, 5, 5> m;
  ColVector<float, 5> v;
  for (std::size_t i = 0; i < 5; i++) {
    m(i,i) = 1;
    v(i) = static_cast<float>(i);
  }

  std::cout << m * v << std::endl;

  auto sq = Matrix<double, 2, 2>::Identity();
  ColVector<double, 2> u {
    3.0,
    4.0
  };
  std::cout << sq * u + u << std::endl;

  Matrix<double, 2, 2> a {
    1.0, 2.0,
    2.0, 1.0
  };
  Matrix<double, 2, 2> b {
    1.0, 0.0,
    0.0, 1.0
  };
  std::cout << a * b << std::endl;
  std::cout << a + b << std::endl;
  std::cout << a - b << std::endl;
  std::cout << a / 3 << std::endl;
  a *= 3;
  b /= 2;
  std::cout << a + b << std::endl;

  auto rmat = Matrix<double, 3, 3>::Random();
  std::cout << 2 * rmat - Matrix<double, 3, 3>::Ones() << std::endl;
  rmat *= rmat;
  std::cout << rmat << std::endl;
  rmat += rmat;
  std::cout << rmat << std::endl;
  rmat -= rmat;
  std::cout << rmat << std::endl;

  Matrix<int8_t, 2, 2> cmat(
    'a','b',
    'c','d'
  );
  std::cout << cmat * static_cast<int8_t>(2) << std::endl;
  cmat *= 2;
  cmat += cmat;
  cmat -= cmat;
  cmat - cmat + cmat * cmat;
  std::cout << cmat + cmat << std::endl;
  Matrix<int8_t, 2, 2> foo(
    'A', 'B',
    'C', 'D'
  );
  std::cout << foo * 2 << std::endl;
  std::cout << foo * 1.15 << std::endl;
  foo *= 0.75;
  std::cout << foo << std::endl;
  foo += 1.3;
  std::cout << foo << std::endl;
  std::cout << foo.Transpose() << std::endl;

  auto var1 = CreateVariable(Matrix<double, 3, 3>::Random());
  std::cout << var1 << std::endl;

  auto var2 = CreateVariable(
      2 * Matrix<double, 3, 3>::Identity()
  );

  Source<Matrix<double, 3, 3>> e1(var1);
  Source<Matrix<double, 3, 3>> e2(var2);
  std::cout << e1() << std::endl;
  std::cout << e2() << std::endl;
  auto e3 = e1 * e2;
  std::cout << e3() << std::endl;
  var2()(0,0) = 4;
  std::cout << e3() << std::endl;
}
