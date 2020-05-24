#include <base/color.hpp>
#include <base/functions.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <display/displayOpenCV.h>
#include <eigen3/Eigen/Core>

int main() {

#ifdef NDEBUG
  constexpr int MAX_ITERATIONS = 512;
#else
  constexpr int MAX_ITERATIONS = 100;
#endif

  disp::DisplayOpenCV D;

  D.setMandelbrotIterations(MAX_ITERATIONS);
  D.setDrawFunction(disp::Display::COLORING::SPLINE);
  // D.setDrawFunction(disp::Display::COLORING::COS);
  D.setNumThreads(4);

  D.startUpdateLoop();

  std::getchar();

  return 0;
}
