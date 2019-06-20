#include <mandelbrot/mandelbrot.h>
#include <vector>

Mandelbrot::Mandelbrot() { initRedistributionSpline(); }

unsigned int Mandelbrot::mandelbrot(const Eigen::Vector2d &position) {
  Eigen::Vector2d Zn(0.0, 0.0);
  for (unsigned int i = 0; i < max_iterations; i++) {
    mandelbrotIteration(position, Zn);
    if (Zn.x() * Zn.x() + Zn.y() * Zn.y() > 4) {
      return i;
    }
  }
  return max_iterations;
}

void Mandelbrot::mandelbrotIteration(const Eigen::Vector2d &position,
                                     Eigen::Vector2d &Zn) {
  Zn = Eigen::Vector2d(Zn.x() * Zn.x() - Zn.y() * Zn.y(), 2 * Zn.x() * Zn.y()) +
       position;
}

unsigned int Mandelbrot::mandelbrot(int x, int y) {
  const Eigen::Vector2d P(x, y);
  return mandelbrot(P);
}

void Mandelbrot::setMaxIterations(unsigned int maxIt) {
  max_iterations = maxIt;
  inv_max_iterations_d = 1. / static_cast<double>(maxIt);
  initRedistributionSpline();
}

void Mandelbrot::mandelbrotGreyScale(unsigned int iterations,
                                     color::RGB<int> &rgb) {
  const double iterations_d = static_cast<double>(iterations);
  const double result = iterations_d * 255. * inv_max_iterations_d;

  rgb.r = rgb.g = rgb.b = func::round(result);
}

void Mandelbrot::initRedistributionSpline() {

  std::vector<double> X(4), Y(4);
  unsigned int i = 0;

  X[i] = 0;
  Y[i++] = 0;

  X[i] = max_iterations * 0.2;
  Y[i++] = max_iterations * 0.4;

  X[i] = max_iterations * 0.5;
  Y[i++] = max_iterations * 0.7;

  // X[i] = max_iterations * 0.75;
  // Y[i++] = max_iterations * 0.70;

  X[i] = max_iterations;
  Y[i++] = max_iterations;

  redistribution_spline.set_points(X, Y);
}

bool Mandelbrot::setSpline(const EigenSTL::vector_Vector2d &spline_points) {

  if (spline_points.size() < 3) {
    return false;
  }

  std::vector<double> X, Y;
  for (const auto sp : spline_points) {
    X.push_back(sp.x());
    Y.push_back(sp.y());
  }

  std::sort(X.begin(), X.end());
  std::sort(Y.begin(), Y.end());

  redistribution_spline.set_points(X, Y);
  return true;
}

double Mandelbrot::redistributeHue(unsigned int iteration) {

  return redistribution_spline(iteration);
}

void Mandelbrot::mandelbrotHSV(unsigned int iterations, color::HSV<int> &hsv) {

  // first redistribute the iterations which represent the hue value to enhance
  // some colors and reduce others

  const double redistributed_iterations = redistributeHue(iterations);

  // use hue to "go smoothely trough each color"
  // hue is between red = 0° -> 45° pink -> 120° blue -> 240° green -> 300°
  // yellow -> 360° red

  const double iterations_d = redistributed_iterations;
  const double iterations_0_255 = iterations_d * 255. * inv_max_iterations_d;
  // this could be a slider "rotateing the color wheel"
  const double rotate_deg = 120.;
  const double rotated_iterations_0_255 =
      iterations_0_255 + (rotate_deg / 360. * 255.);

  // we might have rotted aover 255
  const int clipped_rotated_result =
      func::hsvNormalize255(func::round(rotated_iterations_0_255));
  // hsv.h = clipped_rotated_result;
  hsv.h = 255 - clipped_rotated_result;

  // instead a fade to green a fade to white gives nice kontrasts
  // between 50° and 180°
  // could be sliders
  const double start_fade_white_deg = 50;
  const double start_fade_white = start_fade_white_deg * 255. / 360.;
  const double end_fade_white_deg = 180.;
  const double end_fade_white = end_fade_white_deg * 255. / 360.;
  const double white_fadeing_range_halfe =
      (end_fade_white - start_fade_white) * 0.5;
  const double center_white_fadeing_range =
      white_fadeing_range_halfe + start_fade_white;

  if (hsv.h > start_fade_white) {
    if (hsv.h < center_white_fadeing_range) {
      // fade to white
      const double gradient = 255. / white_fadeing_range_halfe;
      hsv.s = func::round(gradient * (center_white_fadeing_range - hsv.h));
    } else if (hsv.h < end_fade_white) {
      // fade to color
      const double gradient = 255. / white_fadeing_range_halfe;
      hsv.s = func::round(gradient * (hsv.h - center_white_fadeing_range));
    } else {
      hsv.s = 255;
    }
  } else {
    hsv.s = 255;
  }

  // now a very nice touch: a fade to black when aproaching 0 iterations or
  // max iterations

  const double clipping_degree = 20.; // this could be a slider
  const double fadeing_area = 255. * clipping_degree / 360.;
  const double gradient = 255. / fadeing_area;

  if (iterations_0_255 < fadeing_area) {
    // fade to black when close to 0 iterations
    hsv.v = func::round(gradient * iterations_0_255);
  } else if (iterations_0_255 > 255 - fadeing_area) {
    // fade to black when closer to 255
    hsv.v = func::round(gradient * (255 - iterations_0_255));
  } else {
    hsv.v = 255;
  }
}
