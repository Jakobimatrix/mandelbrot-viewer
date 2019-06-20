#ifndef CONVERSATIONS_H
#define CONVERSATIONS_H

#include <base/constants.hpp>
#include <base/conversations.h>
#include <base/structs.hpp>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>

namespace conv {

template <class T> T rad2deg(T rad) {
  return (rad * static_cast<T>(180.0) / static_cast<T>(M_PI));
}
// explicitly instantiate the template otherwize it wont be avaiable
template double rad2deg<double>(double);
template float rad2deg<float>(float);

template <class T> T deg2rad(T deg) {
  return (deg * static_cast<T>(M_PI) / static_cast<T>(180.0));
}
// explicitly instantiate the template otherwize it wont be avaiable
template double deg2rad<double>(double);
template float deg2rad<float>(float);

cv::Point2d EigenToCv(const Eigen::Vector2d &p) {
  return cv::Point2d(p.x(), p.y());
}

cv::Point3d EigenToCv(const Eigen::Vector3d &p) {
  return cv::Point3d(p.x(), p.y(), p.z());
}

Eigen::Vector2d CvToEigen(const cv::Point2d &p) {
  return Eigen::Vector2d(p.x, p.y);
}

Eigen::Vector3d CvToEigen(const cv::Point3d &p) {
  return Eigen::Vector3d(p.x, p.y, p.z);
}
} // namespace conv

#endif
