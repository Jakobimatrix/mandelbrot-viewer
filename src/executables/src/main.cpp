#include <base/color.hpp>
#include <base/functions.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <display/displayOpenCV.h>
#include <eigen3/Eigen/Core>

int main() {
  disp::DisplayOpenCV D;
  //  D.setDrawFunction(disp::Display::COLORING::SPLINE);
  D.setDrawFunction(disp::Display::COLORING::COS);
  D.setNumThreads(4);

  D.startUpdateLoop();

  std::getchar();

  return 0;
}
