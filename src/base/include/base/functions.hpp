#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <algorithm> // std::transform
#include <array>
#include <base/constants.hpp>
#include <base/macros.hpp>
#include <cmath>
#include <functional> // std::multiplies
#include <type_traits>

namespace func {

static int round(double v) { return static_cast<int>(std::round(v)); }

// https://stackoverflow.com/questions/26434128/how-to-make-is-arithmeticmyclassvalue-to-be-true
template <class...> struct make_void { using type = void; };
template <class... Ts> using void_t = typename make_void<Ts...>::type;

template <class T, class = void>
struct supports_arithmetic_operations : std::false_type {};

template <class T>
struct supports_arithmetic_operations<
    T, void_t<decltype(std::declval<T>() + std::declval<T>()),
              decltype(std::declval<T>() - std::declval<T>()),
              decltype(std::declval<T>() * std::declval<T>()),
              decltype(std::declval<T>() / std::declval<T>())>>
    : std::true_type {};

namespace math_detail {

template <typename T1, typename T2>
auto positiveModulo(T1 x, T2 y, std::true_type, std::false_type) {
  static_assert(std::is_signed<T1>::value && std::is_signed<T2>::value,
                "you passed an unsigned value to positiveModulo. "
                "positiveModulo only works with signed integer or floating "
                "point parameters");
  // see
  // https://stackoverflow.com/questions/14997165/fastest-way-to-get-a-positive-modulo-in-c-c
  return (x % y + y) % y;
}

template <typename T1, typename T2>
auto positiveModulo(T1 x, T2 y, std::false_type, std::true_type) {
  // see https://stackoverflow.com/questions/11980292/how-to-wrap-around-a-range
  return x - std::floor(x / y) * y;
}

} // namespace math_detail

template <typename T1, typename T2> auto positiveModulo(T1 x, T2 y) {
  return math_detail::positiveModulo(
      x, y, typename std::integral_constant < bool,
      std::is_integral<T1>::value &&std::is_integral<T2>::value > (),
      typename std::integral_constant < bool,
      std::is_floating_point<T1>::value ||
          std::is_floating_point<T2>::value > ());
}

template <class T> void minMax(T &x1, T &x2) {
  if (x1 > x2) {
    std::swap(x1, x2);
  }
}

template <class T> bool isBetween(T x, T min, T max) {
  minMax(min, max);
  if (min < x && x < max) {
    return true;
  }
  return false;
}

/*!
 * \brief wraps the given angle into the range [given-Pi, given+Pi).
 * \param angle The angle to wrap.
 * \param given The angle to wrap around.
 * \return the wraped angle.
 */
template <typename T> T wrapAngleAroundGivenAngle(T angle, T given) {
  static_assert(
      std::is_floating_point<T>::value,
      "you can only wrap floating point angles (float, double, long double)");
  return positiveModulo(angle - given + M_PI, 2 * M_PI) - M_PI + given;
}

/*!
 * \brief wraps the given angle into the range [given-180, given+180).
 * \param angle The angle to wrap.
 * \param given The angle to wrap around.
 * \return the wraped angle.
 */
template <typename T> T wrapAngleAroundGivenAngleDeg(T angle, T given) {
  static_assert(
      std::is_floating_point<T>::value,
      "you can only wrap floating point angles (float, double, long double)");
  return positiveModulo(angle - given + 180, 360) - 180 + given;
}

template <class T> T clip255Mirror(T value) {
  return positiveModulo(value, 255.0);
}

template <class T> T clip1Mirror(T value) { return positiveModulo(value, 1); }

static int hsvNormalize255(int value) {
  if (value > 255) {
    const int div = value / 255; //.round(static_cast<double>(value) / 255.);
    value = value - 255 * div;
  }
  return value;
}

static double hsvNormalize01(double value) {
  if (value > 1) {
    value = hsvNormalize01(value - 1.);
  } else if (value < 0) {
    value = hsvNormalize01(value + 1.);
  }
  return value;
}

template <class T> T clip255MinMax(T value) {
  if (value < 0) {
    return static_cast<T>(0);
  }
  if (value > 255) {
    return static_cast<T>(255);
  }
  return value;
}

/// array operationen
// static cast
// http://loungecpp.wikidot.com/tips-and-tricks%3aindices
template <std::size_t... Is> struct indices {};
template <std::size_t N, std::size_t... Is>
struct build_indices : build_indices<N - 1, N - 1, Is...> {};
template <std::size_t... Is> struct build_indices<0, Is...> : indices<Is...> {};

template <typename T, typename U, size_t i, size_t... Is>
constexpr auto array_cast_helper(const std::array<U, i> &a, indices<Is...>)
    -> std::array<T, i> {
  return {{static_cast<T>(std::get<Is>(a))...}};
}

template <typename T, typename U, size_t i>
constexpr auto array_cast(const std::array<U, i> &a) -> std::array<T, i> {
  // tag dispatch to helper with array indices
  return array_cast_helper<T>(a, build_indices<i>());
}

// define functions for basic math operators O
template <typename T> constexpr T mult(T const &a, T const &b) { return a * b; }
template <typename T> constexpr T div(T const &a, T const &b) { return a / b; }
template <typename T> constexpr T add(T const &a, T const &b) { return a + b; }
template <typename T> constexpr T subst(T const &a, T const &b) {
  return a - b;
}

// element wise array1[i] * array1[i]
template <class T, size_t... Is, size_t N>
constexpr std::array<T, N>
multiplyEntryWise(std::array<T, N> const &src, std::array<T, N> const &src2,
                  std::index_sequence<Is...> = std::make_index_sequence<N>{}) {
  return std::array<T, N>{{mult(src[Is], src2[Is])...}};
}

// element wise array1[i] / array1[i]
template <class T, size_t... Is, size_t N>
constexpr std::array<T, N>
divideEntryWise(std::array<T, N> const &src, std::array<T, N> const &src2,
                std::index_sequence<Is...> = std::make_index_sequence<N>{}) {
  return std::array<T, N>{{div(src[Is], src2[Is])...}};
}

// element wise array1[i] + array1[i]
template <class T, size_t... Is, size_t N>
constexpr std::array<T, N>
addEntryWise(std::array<T, N> const &src, std::array<T, N> const &src2,
             std::index_sequence<Is...> = std::make_index_sequence<N>{}) {
  return std::array<T, N>{{add(src[Is], src2[Is])...}};
}

// element wise array1[i] - array1[i]
template <class T, size_t... Is, size_t N>
constexpr std::array<T, N>
substEntryWise(std::array<T, N> const &src, std::array<T, N> const &src2,
               std::index_sequence<Is...> = std::make_index_sequence<N>{}) {
  return std::array<T, N>{{subst(src[Is], src2[Is])...}};
}

// element wise array1[i] * x
template <class T, size_t... Is, size_t N>
constexpr std::array<T, N>
multiplyEntryWise(std::array<T, N> const &src, T const &x,
                  std::index_sequence<Is...> = std::make_index_sequence<N>{}) {
  return std::array<T, N>{{mult(src[Is], x)...}};
}

// element wise array1[i] / x
template <class T, size_t... Is, size_t N>
constexpr std::array<T, N>
divideEntryWise(std::array<T, N> const &src, T const &x,
                std::index_sequence<Is...> = std::make_index_sequence<N>{}) {
  return std::array<T, N>{{mult(src[Is], x)...}};
}

// element wise x / array1[i]
template <class T, size_t... Is, size_t N>
constexpr std::array<T, N>
divideEntryWise(T const &x, std::array<T, N> const &src,
                std::index_sequence<Is...> = std::make_index_sequence<N>{}) {
  return std::array<T, N>{{mult(x, src[Is])...}};
}

// element wise array1[i] + x
template <class T, size_t... Is, size_t N>
constexpr std::array<T, N>
addEntryWise(std::array<T, N> const &src, T const &x,
             std::index_sequence<Is...> = std::make_index_sequence<N>{}) {
  return std::array<T, N>{{mult(src[Is], x)...}};
}

// element wise array1[i] - x
template <class T, size_t... Is, size_t N>
constexpr std::array<T, N>
substEntryWise(std::array<T, N> const &src, T const &x,
               std::index_sequence<Is...> = std::make_index_sequence<N>{}) {
  return std::array<T, N>{{mult(src[Is], x)...}};
}
} // namespace func

#endif
