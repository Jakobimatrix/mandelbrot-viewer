#include <base/color.hpp>
#include <base/functions.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <display/displayOpenCV.h>
#include <eigen3/Eigen/Core>

int main() {

  disp::DisplayOpenCV D = disp::DisplayOpenCV();

  D.setDrawFunction(disp::Display::DRAWING_FUNKTION::MANDELBROT_COLORED);

  D.setNumThreads(4);

  D.startUpdateLoop();

  std::getchar();

  return 0;
}
