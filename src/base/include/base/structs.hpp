#ifndef STRUCTS_H
#define STRUCTS_H
#include <base/functions.hpp>
#include <eigen3/Eigen/Core>
#include <type_traits>

namespace geometry {

struct Rect {
  Eigen::Vector2d corner1;
  Eigen::Vector2d corner2;

  Rect() {}

  double width() const { return corner2.x() - corner1.x(); }

  double height() const { return corner2.y() - corner1.y(); }

  Eigen::Vector2d size() const { return Eigen::Vector2d(width(), height()); }

  Eigen::Vector2d center() const { return corner1 + (corner2 - corner1) * 0.5; }

  void clean() {
    func::minMax(corner1.x(), corner2.x());
    func::minMax(corner1.y(), corner2.y());
  }

  void setCenter(const Eigen::Vector2d &center_) {
    const Eigen::Vector2d diff = center_ - center();
    corner1 += diff;
    corner2 += diff;
  }

  Rect(double x1, double y1, double x2, double y2) {
    corner1.x() = x1;
    corner1.y() = y1;
    corner2.x() = x2;
    corner2.y() = y2;
    clean();
  }

  Rect(const Eigen::Vector2d &c1, const Eigen::Vector2d &c2) {
    corner1 = c1;
    corner2 = c2;
    clean();
  }

  Rect(const Eigen::Vector2d &c1, double width, double height) {
    corner1 = c1;
    corner2 = c1 + Eigen::Vector2d(width, height);
    clean();
  }

  Rect(const Rect &rect) {
    corner1 = rect.corner1;
    corner2 = rect.corner2;
    clean();
  }

  Rect &operator=(const Rect &other) {
    corner1 = other.corner1;
    corner2 = other.corner2;
    clean();
  }

  Rect operator&(const Rect &other) const {
    const double x1 = std::max(corner1.x(), other.corner1.x());
    const double y1 = std::max(corner1.y(), other.corner1.y());
    const double x2 = std::max(corner2.x(), other.corner2.x());
    const double y2 = std::max(corner2.y(), other.corner2.y());
    const Rect ret(x1, y1, x2, y2);
    return ret;
  }
};

} // namespace geometry

#endif
