#ifndef CONVERSATIONS_H
#define CONVERSATIONS_H

#include <base/structs.hpp>
#include <eigen3/Eigen/Core>
#include <opencv2/opencv.hpp>

namespace conv {

template <class T> T rad2deg(T rad);

template <class T> T deg2rad(T deg);

cv::Point2d EigenToCv(const Eigen::Vector2d &p);

cv::Point3d EigenToCv(const Eigen::Vector3d &p);

Eigen::Vector2d CvToEigen(const cv::Point2d &p);

Eigen::Vector3d CvToEigen(const cv::Point3d &p);

} // namespace conv

#endif
