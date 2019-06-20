/*
 *
 *
 *
 *
 *
 + Usage:
  std::array<int, 3> pigments_i = {{0, 100, 255}};
  std::array<double, 3> pigments_d = {{0., 0.3921568627, 1.}};

  // initiation with array or 3 single numbers//in order h,s,i
  color::HSV<int> hsvi(pigments_i);
  color::HSV<int> hsvi2(0, 100, 255);
  color::HSV<int> hsvi3(pigments_d);
  color::HSV<int> hsvi4(0., 0.3921568627, 1.);

  // initiation with other class of HSV from type integral or floating point
  color::HSV<double> hsvd(hsvi);

  // initiate RGB with class HSV. Array or 3 single numbers in order r,g,b
  color::RGB<int> rgbii(hsvi);
  color::RGB<int> rgbid(hsvd);
  color::RGB<double> rgbdi(hsvi);
  color::RGB<double> rgbdd(hsvd);

  // assigment between different types and color spaces is possible too.
  color::RGB<int> rgbii2 = rgbdi;
  color::RGB<int> rgbii3 = hsvi;
  color::RGB<int> rgbii4 = hsvd;

  color::RGB<double> rgbdi2 = rgbdi;
  color::RGB<double> rgbdi3 = hsvi;
  color::RGB<double> rgbdi4 = hsvd;

  // output
  std::cout << rgbdi2;

  std::cout << "---hsv2rgb---" << std::endl;

  // conversion to RGB
  std::cout << hsvi << "to:" << std::endl;
  std::cout << color::convertToRGB(hsvi);
  auto x = color::convertToRGB(hsvi);
  std::cout << color::convertToRGB(hsvd);

  std::cout << "---rgb2hsv---" << std::endl;

  // conversion to HSV
  std::cout << x << "to:" << std::endl;
  std::cout << color::convertToHSV(x);
  std::cout << color::HSV<int>(color::convertToHSV(x));
  std::cout << color::convertToHSV(rgbid);

  std::cout << "---hsv2hsv---" << std::endl;

  // since color types get converted into each other within the constructor you
  // could do this too if you are bored:
  std::cout << hsvd;
  hsvd = color::convertToHSV(hsvd);
  std::cout << hsvd;

  std::cout << "---access---" << std::endl;

  // access color data like this:
  std::cout << " H:" << hsvd.h << " S:" << hsvd.s << " V:" << hsvd.v
            << std::endl;
  std::cout << " R:" << rgbid.r << " G:" << rgbid.g << " B:" << rgbid.b
            << std::endl;
  // or this
  for (unsigned int i = 0; i < 3; i++) {
    std::cout << " Data<" << i << ">:" << rgbii2[i] << std::endl;
  }
*/

#ifndef COLOR_H
#define COLOR_H

#include <array>
#include <base/functions.hpp>
#include <cassert>
#include <iostream>
#include <type_traits>

namespace color {

template <class T> class Color {
public:
  constexpr static unsigned int NUM_PIGMENTS = 3;

  std::array<T, NUM_PIGMENTS>
      pigment; // a fraction between 0 and 1 OR int [0 - 255]

protected:
  Color() {}

  template <class T_>
  Color(T_ a, T_ b, T_ c) : Color(std::array<T_, NUM_PIGMENTS>{{a, b, c}}) {}

  template <class T_> Color(const std::array<T_, NUM_PIGMENTS> &pigment_) {
    static_assert(func::supports_arithmetic_operations<T_>::value,
                  "You cannot initiate a color with given type ");

    if (std::is_same<T_, T>::value) {
      // pigment[i] = pigment_[i]; // this doesnt work???
      for (unsigned int i = 0; i < NUM_PIGMENTS; i++) {
        pigment[i] = pigment_[i];
      }
      // pigment = pigment_;
    } else if (isFloatingpoint() && std::is_integral<T_>::value) {
      for (unsigned int i = 0; i < NUM_PIGMENTS; i++) {
        pigment[i] = static_cast<T>(pigment_[i]) / 255.0;
      }
      // pigment = static_cast<T>(pigment_) / 255.0;
    } else if (isIntegral() && std::is_floating_point<T_>::value) {
      for (unsigned int i = 0; i < NUM_PIGMENTS; i++) {
        pigment[i] = std::round(pigment_[i] * 255.0);
      }
    }
  }

public:
  virtual std::string pigmentName(unsigned int) const = 0;
  virtual std::string getColorTypeName() const = 0;

  bool isIntegral() const { return std::is_integral<T>::value; }

  bool isFloatingpoint() const { return std::is_floating_point<T>::value; }

  friend std::ostream &operator<<(std::ostream &os, const Color &c) {
    os << c.getColorTypeName() << std::endl;
    for (unsigned int i = 0; i < NUM_PIGMENTS; i++) {
      os << "[" << c.pigmentName(i) << ": " << c.pigment[i] << "]";
    }
    os << std::endl;
    return os;
  }

  T &operator[](unsigned int x) { return pigment[x]; }

  /*

  template <class T_>
  Color(T_ a, T_ b, T_ c) : Color(std::array<T_, NUM_PIGMENTS>{{a, b, c}}) {}

  Color(const std::array<double, NUM_PIGMENTS> &pigment_) {
    if (std::is_same<T, double>::value) {
      for (unsigned int i = 0; i < NUM_PIGMENTS; i++) {
        pigment[i] = static_cast<T>(pigment_[i]);
      }
      // pigment = pigment_;
    } else if (isFloatingpoint()) {
      for (unsigned int i = 0; i < NUM_PIGMENTS; i++) {
        pigment[i] = static_cast<T>(pigment_[i]) / 255.0;
      }
      // pigment = static_cast<T>(pigment_) / 255.0;
    } else {
      for (unsigned int i = 0; i < NUM_PIGMENTS; i++) {
        pigment[i] = static_cast<T>(pigment_[i]) * 255;
      }
    }
  }

  template <class T_, TYPE def_> Color(const Color<T_, def_> &c) {

    const Color<T, def> temp = c.convert();

    for (unsigned int i = 0; i < NUM_PIGMENTS; i++) {
      pigment[i] = static_cast<T>(temp.pigment_[i]);
    }

    // pigment = temp.pigment;
  }


  template <class T_, TYPE def_> Color<T_, def_> convert() const {
    if (std::is_same<T, T_>::value) {
      // nothing to convert, maybe type which is done by constructore
      return Color<T_, def_>(pigment);
    } else {
      const Color<T, def> temp(pigment);
      switch (def_) {
      case TYPE::HSV:
        return convertToHSV(temp);
      case TYPE::RGB:
        return convertToRGB(temp);
      }
    }
  }

  */
};

template <class T> class RGB;
template <class T> class HSV;

static RGB<double> convertToRGB(const HSV<double> &hsv);
static HSV<double> convertToHSV(const RGB<double> &rgb);

template <class T> class RGB : public Color<T> {
public:
  // https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members
  // We need to tell the compiler explicitly that the names are dependent on the
  // instantiation of the parent. This is because the template parent of a
  // template class is not instantiated during the compilation pass that first
  // examines the template.
  T &r = this->pigment[0];
  T &g = this->pigment[1];
  T &b = this->pigment[2];

  using Color<T>::NUM_PIGMENTS; // same problem as above, we need to tell the
                                // compiler where the constant comes from

  RGB() {}

  template <class T_> RGB(const RGB<T_> &rgb) : Color<T>(rgb.pigment) {}

  template <class T_> RGB(const HSV<T_> &hsv) {
    const RGB<T> temp = convertToRGB(hsv);
    *this = temp;
  }

  template <class T_>
  RGB(const std::array<T_, NUM_PIGMENTS> &pigments) : Color<T>(pigments) {}

  template <class T_>
  RGB(T_ a, T_ b, T_ c) : Color<T>(std::array<T_, NUM_PIGMENTS>{{a, b, c}}) {}

  RGB<T> &operator=(const RGB<T> &rgb) {
    if (&rgb == this) {
      return *this;
    }
    // constructor deals with different T
    const RGB<T> temp(rgb);
    std::copy(&temp.pigment[0], &temp.pigment[0] + NUM_PIGMENTS,
              &this->pigment[0]);
    return *this;
  }

  template <class T_> RGB<T> &operator=(const HSV<T_> &hsv) {
    const RGB<T> temp = convertToRGB(hsv);
    *this = temp;
    return *this;
  }

  std::string pigmentName(unsigned int i) const override {
    switch (i) {
    case 0:
      return "R";
    case 1:
      return "G";
    case 2:
      return "B";
    }
  }

  std::string getColorTypeName() const override {
    if (std::is_same<T, int>::value) {
      return "RGB_integer";
    } else {
      return "RGB_double";
    }
  }
};

template <class T> class HSV : public Color<T> {
public:
  T &h = this->pigment[0];
  T &s = this->pigment[1];
  T &v = this->pigment[2];

  using Color<T>::NUM_PIGMENTS; // same problem as above, we need to tell the
                                // compiler where the constant comes from

  HSV() {}

  template <class T_> HSV(const HSV<T_> &hsv) : Color<T>(hsv.pigment) {}

  template <class T_> HSV(const RGB<T_> &rgb) {
    const HSV<T> temp = convertToHSV(rgb);
    *this = temp;
  }

  template <class T_>
  HSV(const std::array<T_, NUM_PIGMENTS> &pigments) : Color<T>(pigments) {}

  template <class T_>
  HSV(T_ a, T_ b, T_ c) : Color<T>(std::array<T_, NUM_PIGMENTS>{{a, b, c}}) {}

  HSV<T> &operator=(const HSV<T> &hsv) {
    if (&hsv == this) {
      return *this;
    }
    // constructor deals with different T
    const HSV<T> temp(hsv);
    std::copy(&temp.pigment[0], &temp.pigment[0] + NUM_PIGMENTS,
              &this->pigment[0]);
    return *this;
  }

  template <class T_> HSV<T> &operator=(const RGB<T_> &rgb) {
    const HSV<T> temp = convertToHSV(rgb);
    *this = temp;
    return *this;
  }

  std::string pigmentName(unsigned int i) const override {
    switch (i) {
    case 0:
      return "H";
    case 1:
      return "S";
    case 2:
      return "V";
    }
  }

  std::string getColorTypeName() const override {
    if (std::is_same<T, int>::value) {
      return "HSV_integer";
    } else {
      return "HSV_double";
    }
  }
};

// https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
static RGB<double> convertToRGB(const HSV<double> &hsv) {

  if (hsv.s <= 0.0) { // < is bogus, just shuts up warnings
    return RGB<double>(hsv.v, hsv.v, hsv.v);
  }

  const double hh = hsv.h >= 1.0 ? 0.0 : hsv.h * 6.;

  const long region = static_cast<long>(hh);
  const double remainder = hh - region;
  const double p = hsv.v * (1.0 - hsv.s);
  const double q = hsv.v * (1.0 - (hsv.s * remainder));
  const double t = hsv.v * (1.0 - (hsv.s * (1.0 - remainder)));

  RGB<double> rgb;

  switch (region) {
  case 0:
    rgb.r = hsv.v;
    rgb.g = t;
    rgb.b = p;
    break;
  case 1:
    rgb.r = q;
    rgb.g = hsv.v;
    rgb.b = p;
    break;
  case 2:
    rgb.r = p;
    rgb.g = hsv.v;
    rgb.b = t;
    break;
  case 3:
    rgb.r = p;
    rgb.g = q;
    rgb.b = hsv.v;
    break;
  case 4:
    rgb.r = t;
    rgb.g = p;
    rgb.b = hsv.v;
    break;
  case 5:
  default:
    rgb.r = hsv.v;
    rgb.g = p;
    rgb.b = q;
    break;
  }

  return rgb;
}

static HSV<double> convertToHSV(const RGB<double> &rgb) {
  HSV<double> hsv;
  double min, max, delta;

  min = rgb.r < rgb.g ? rgb.r : rgb.g;
  min = min < rgb.b ? min : rgb.b;

  max = rgb.r > rgb.g ? rgb.r : rgb.g;
  max = max > rgb.b ? max : rgb.b;

  hsv.v = max; // v
  delta = max - min;
  if (delta < 0.00001) {
    hsv.s = 0;
    hsv.h = 0; // undefined, maybe nan?
    return hsv;
  }
  if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
    hsv.s = (delta / max); // s
  } else {
    // if max is 0, then r = g = b = 0
    // s = 0, h is undefined
    hsv.s = 0.0;
    hsv.h = NAN; // its now undefined
    return hsv;
  }
  if (rgb.r >= max)                  // > is bogus, just keeps compilor happy
    hsv.h = (rgb.g - rgb.b) / delta; // between yellow & magenta
  else if (rgb.g >= max)
    hsv.h = 2.0 + (rgb.b - rgb.r) / delta; // between cyan & yellow
  else
    hsv.h = 4.0 + (rgb.r - rgb.g) / delta; // between magenta & cyan

  hsv.h /= 6.0; // degrees between 0 and 1

  if (hsv.h < 0.0)
    hsv.h += 1.0;

  return hsv;
}

} // namespace color

#endif
