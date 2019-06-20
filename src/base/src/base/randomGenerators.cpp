#include <base/randomGenerators.h>

namespace func {

////////////////////////
// GLOBAL VARIABLES
////////////////////////
std::random_device rd;
std::default_random_engine generator(rd());

int uniform_int_dist(int min, int max) {
  if (min > max) {
    std::uniform_int_distribution<int> distribution(max, min);
    return distribution(generator);
  } else {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
  }
}

bool randTrueFalse() {
  std::uniform_int_distribution<int> distribution(0, 1);
  if (distribution(generator) == 1) {
    return true;
  } else {
    return false;
  }
}

double uniform_double_dist(double min, double max) {
  if (min > max) {
    std::uniform_real_distribution<> distribution(0.0f, 1.0f);
    return max + distribution(generator) * (min - max);
  } else {
    std::uniform_real_distribution<> distribution(0.0f, 1.0f);
    return min + distribution(generator) * (max - min);
  }
}

double randGaus(double mean, double variance) {
  std::normal_distribution<double> distribution(mean, variance);
  return distribution(generator);
}

} // namespace func
