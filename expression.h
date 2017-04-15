#ifndef CGRAPH_EXPRESSION_H
#define CGRAPH_EXPRESSION_H

#include "variable.h"

#include <functional>
#include <tuple>
#include <type_traits>

namespace cgraph {

namespace {
  template <typename F, typename ...Args, std::size_t ... Indices>
  auto apply_impl(F &&f, const std::tuple<Args...> &tuple,
                  std::index_sequence<Indices...>) {
    return std::forward<F>(f)(std::get<Indices>(tuple)...);
  }

  template <typename F, typename ...Args>
  auto apply(F &&f, const std::tuple<Args...> &t) {
    return apply_impl(
        std::forward<F>(f), t, std::index_sequence_for<Args...>()
    );
  }
}

// Expression resulting from an n-ary function.
template<typename R, typename... Args>
class Expression {
public:
  // Output type is based on function.
  using OutputType = R;

  // Construct the expression.
  Expression(std::function<R(Args...)> ff, Args... aa)
      : f(ff), args(aa...) { }

  // Evaluate the expression.
  OutputType operator()() const {
    return apply(f, args);
  }
private:
  std::function<R(Args...)> f;
  std::tuple<Args...> args;
};

// Expression that simply wraps a variable.
template<typename T>
class Expression<Variable<T>> {
public:
  // Output type is just a T.
  using OutputType = T;

  // Construct the expression.
  Expression(Variable<T> &v) : var(v) {}

  // Access the variable.
  OutputType &operator()() {
    return var();
  }
  const OutputType &operator()() const {
    return var();
  }
private:
  Variable<T> &var;
};

auto binary_multiplier = [](const auto &expr1, const auto &expr2)
    -> decltype(expr1() * expr2()) {
  return expr1() * expr2();
};

auto binary_adder = [](const auto &expr1, const auto &expr2)
    -> decltype(expr1() * expr2()) {
  return expr1() + expr2();
};

auto binary_subtracter = [](const auto &expr1, const auto &expr2)
    -> decltype(expr1() * expr2()) {
  return expr1() - expr2();
};

auto binary_divider = [](const auto &expr1, const auto &expr2)
    -> decltype(expr1() * expr2()) {
  return expr1() / expr2();
};

template<typename ...T, typename ...R>
auto operator*(const Expression<T...> &e1, const Expression<R...> &e2) {
  using RT = decltype(binary_multiplier(e1, e2));
  using E1 = decltype(e1);
  using E2 = decltype(e2);
  std::function<RT(E1, E2)> func(binary_multiplier);
  return Expression<RT, E1, E2>(func, e1, e2);
}

template<typename ...T, typename ...R>
auto operator+(const Expression<T...> &e1, const Expression<R...> &e2) {
  using RT = decltype(binary_adder(e1, e2));
  using E1 = decltype(e1);
  using E2 = decltype(e2);
  std::function<RT(E1, E2)> func(binary_adder);
  return Expression<RT, E1, E2>(func, e1, e2);
}

template<typename ...T, typename ...R>
auto operator-(const Expression<T...> &e1, const Expression<R...> &e2) {
  using RT = decltype(binary_subtracter(e1, e2));
  using E1 = decltype(e1);
  using E2 = decltype(e2);
  std::function<RT(E1, E2)> func(binary_subtracter);
  return Expression<RT, E1, E2>(func, e1, e2);
}

template<typename ...T, typename ...R>
auto operator/(const Expression<T...> &e1, const Expression<R...> &e2) {
  using RT = decltype(binary_divider(e1, e2));
  using E1 = decltype(e1);
  using E2 = decltype(e2);
  std::function<RT(E1, E2)> func(binary_divider);
  return Expression<RT, E1, E2>(func, e1, e2);
}

template<typename T>
using Source = Expression<Variable<T>>;

}

#endif // CGRAPH_EXPRESSION_H
