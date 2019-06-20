#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/StdVector>
#include <vector>

namespace EigenSTL {
typedef Eigen::Matrix<double, 8, 1> Vector8d;
typedef std::vector<Eigen::Vector2d, Eigen::aligned_allocator<Eigen::Vector2d>> vector_Vector2d;
typedef std::pair<Eigen::Vector2d,Eigen::Vector2d> pair_Vector2d_Vector2d;
typedef std::vector<pair_Vector2d_Vector2d, Eigen::aligned_allocator<pair_Vector2d_Vector2d>> vector_pair_Vector2d_Vector2d;
}
