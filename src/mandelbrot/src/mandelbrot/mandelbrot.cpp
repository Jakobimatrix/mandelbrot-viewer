#include <mandelbrot/mandelbrot.h>
#include <vector>

Mandelbrot::Mandelbrot() { initRedistributionSpline(); }

double Mandelbrot::mandelbrot(const Eigen::Vector2d &position) const {
  if (smooting) {
    return mandelbrot_smooth(position);
  }
  return mandelbrot_classic(position);
}

double Mandelbrot::mandelbrot(int x, int y) const {
  const Eigen::Vector2d P(x, y);
  return mandelbrot(P);
}

double Mandelbrot::mandelbrot_classic(const Eigen::Vector2d &position) const {
  if (isInsideM1M2(position)) {
    return 0;
  }
  Eigen::Vector2d Zn(0.0, 0.0);
  const double G = 4;

  double i = 0;
  for (; i < max_iterations; i++) {
    mandelbrotIteration(position, Zn);
    if (Zn.dot(Zn) > G) {
      break;
    }
  }
  return i;
}

double Mandelbrot::mandelbrot_smooth(const Eigen::Vector2d &position) const {
  if (isInsideM1M2(position)) {
    return 0;
  }
  Eigen::Vector2d Zn(0.0, 0.0);
  const double G = 256.0 * 256.0; // 2*2

  double i = 0.;
  for (; i < max_iterations; i++) {
    mandelbrotIteration(position, Zn);
    if (Zn.dot(Zn) > G) {
      break;
    }
  }

  if (i > max_iterations - 1)
    return 0;

  // smoothing
  i = (i - std::log2(std::log2(Zn.dot(Zn))) + 4.0) * max_iterations;
  return i;
}

bool Mandelbrot::isInsideM1M2(const Eigen::Vector2d &position) const {
  const double c2 = position.dot(position);
  // skip computation inside M1 -
  // http://iquilezles.org/www/articles/mset_1bulb/mset1bulb.htm
  if (256.0 * c2 * c2 - 96.0 * c2 + 32.0 * position.x() - 3.0 < 0.0)
    return true;
  // skip computation inside M2 -
  // http://iquilezles.org/www/articles/mset_2bulb/mset2bulb.htm
  if (16.0 * (c2 + 2.0 * position.x() + 1.0) - 1.0 < 0.0)
    return true;

  return false;
}

void Mandelbrot::setSmoothing(bool s) { smooting = s; }

bool Mandelbrot::getSmoothing() const { return smooting; }

void Mandelbrot::mandelbrotIteration(const Eigen::Vector2d &position,
                                     Eigen::Vector2d &Zn) {
  Zn = Eigen::Vector2d(Zn.x() * Zn.x() - Zn.y() * Zn.y(), 2 * Zn.x() * Zn.y()) +
       position;
}

void Mandelbrot::setMaxIterations(unsigned int maxIt) {
  max_iterations = maxIt;
  inv_max_iterations_d = 1. / static_cast<double>(maxIt);
  initRedistributionSpline();
}

void Mandelbrot::mandelbrotGreyScale(double iterations, color::RGB<int> &rgb) {
  const double result = iterations * 255. * inv_max_iterations_d;
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

double Mandelbrot::redistributeHue(double iteration) {

  return redistribution_spline(iteration);
}

color::RGB<double> Mandelbrot::mandelbrotCOS(double iterations) {
  color::RGB<double> rgb;
  const double a = 3.0 + iterations * 0.15;
  rgb.r = 0.5 + 0.5 * std::cos(a);
  rgb.g = 0.5 + 0.5 * std::cos(a + 0.6);
  rgb.b = 0.5 + 0.5 * std::cos(a + 1.0);
  return rgb;
}

color::HSV<double> Mandelbrot::mandelbrotSPLINE(double iterations) {
  color::HSV<double> hsv;

  // first redistribute the iterations which represent the hue value to
  // enhance some colors and reduce others

  const double redistributed_iterations = redistributeHue(iterations);

  // use hue to "go smoothely trough each color"
  // hue is between red = 0° -> 45° pink -> 120° blue -> 240° green -> 300°
  // yellow -> 360° red

  const double iterations_0_255 =
      redistributed_iterations * inv_max_iterations_d;
  // this could be a slider "rotateing the color wheel"
  const double rotate_deg = 120.;
  const double rotated_iterations_0_255 =
      iterations_0_255 + (rotate_deg / 360.);

  // we might have rotted over 255
  const double clipped_rotated_result =
      func::hsvNormalize01(func::round(rotated_iterations_0_255));
  // hsv.h = clipped_rotated_result;
  hsv.h = 1 - clipped_rotated_result;

  // instead a fade to green a fade to white gives nice kontrasts
  // between 50° and 180°
  // could be sliders
  const double start_fade_white_deg = 50;
  const double start_fade_white = start_fade_white_deg / 360.;
  const double end_fade_white_deg = 180.;
  const double end_fade_white = end_fade_white_deg / 360.;
  const double white_fadeing_range_halfe =
      (end_fade_white - start_fade_white) * 0.5;
  const double center_white_fadeing_range =
      white_fadeing_range_halfe + start_fade_white;

  if (hsv.h > start_fade_white) {
    if (hsv.h < center_white_fadeing_range) {
      // fade to white
      const double gradient = 1. / white_fadeing_range_halfe;
      hsv.s = func::round(gradient * (center_white_fadeing_range - hsv.h));
    } else if (hsv.h < end_fade_white) {
      // fade to color
      const double gradient = 1. / white_fadeing_range_halfe;
      hsv.s = func::round(gradient * (hsv.h - center_white_fadeing_range));
    } else {
      hsv.s = 1;
    }
  } else {
    hsv.s = 1.;
  }

  // now a very nice touch: a fade to black when aproaching 0 iterations or
  // max iterations

  const double clipping_degree = 20.; // this could be a slider
  const double fadeing_area = clipping_degree / 360.;
  const double gradient = fadeing_area;

  if (iterations_0_255 < fadeing_area) {
    // fade to black when close to 0 iterations
    hsv.v = func::round(gradient * iterations_0_255);
  } else if (iterations_0_255 > 1. - fadeing_area) {
    // fade to black when closer to 255
    hsv.v = func::round(gradient * (1. - iterations_0_255));
  } else {
    hsv.v = 1;
  }
}
