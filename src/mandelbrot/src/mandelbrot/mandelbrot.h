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

  unsigned int mandelbrot(const Eigen::Vector2d &position);
  unsigned int mandelbrot(int, int);

  void mandelbrotGreyScale(unsigned int iterations, color::RGB<int> &rgb);
  void mandelbrotHSV(unsigned int iterations, color::HSV<int> &hsv);

  double redistributeHue(unsigned int iteration);

  void setMaxIterations(unsigned int maxIt);

  void initRedistributionSpline();

  bool setSpline(const EigenSTL::vector_Vector2d &splinePoints);

private:
  static void mandelbrotIteration(const Eigen::Vector2d &poition,
                                  Eigen::Vector2d &Zn);

  unsigned int max_iterations = 255;
  double inv_max_iterations_d = 1. / 255.;

  tk::spline redistribution_spline;
};

#endif
