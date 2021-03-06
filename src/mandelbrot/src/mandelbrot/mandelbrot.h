#ifndef MANDELBROETCHEN_H
#define MANDELBROETCHEN_H

#include <base/color.hpp>
#include <base/structs.hpp>
#include <base/typedefs.hpp>
#include <eigen3/Eigen/Core>
#include <spline.h>

class Mandelbrot {
public:
  Mandelbrot();
  ~Mandelbrot() {}

  double mandelbrot(const Eigen::Vector2d &position) const;
  double mandelbrot(int, int) const;
  double mandelbrot_classic(const Eigen::Vector2d &position) const;
  double mandelbrot_smooth(const Eigen::Vector2d &position) const;
  bool isInsideM1M2(const Eigen::Vector2d &position) const;

  void mandelbrotGreyScale(double iterations, color::RGB<int> &rgb);
  color::HSV<double> mandelbrotSPLINE(double iterations);
  color::RGB<double> mandelbrotCOS(double iterations);

  double redistributeHue(double iteration);

  void setMaxIterations(unsigned int maxIt);

  unsigned int getMaxIterations() const { return max_iterations; }

  void initRedistributionSpline();

  bool setSpline(const EigenSTL::vector_Vector2d &splinePoints);

  void setCosParams(double a, double b, double c, double d);

  void setSmoothing(bool s);

  bool getSmoothing() const;

private:
  static void mandelbrotIteration(const Eigen::Vector2d &poition,
                                  Eigen::Vector2d &Zn);

  unsigned int max_iterations = 0;
  double inv_max_iterations_d = 0.;

  tk::spline redistribution_spline;

  double cos_const_a = 0.069; // 0.15;
  double cos_const_b = 0;
  double cos_const_c = 0.6;
  double cos_const_d = 1;

  bool smooting = false;
};

#endif
