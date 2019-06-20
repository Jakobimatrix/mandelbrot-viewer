#ifndef PLANAR_TRANSFORMATION_H
#define PLANAR_TRANSFORMATION_H

#include <base/structs.hpp>
#include <eigen3/Eigen/Core>
#include <vector>

namespace conv {

class PlanarTransformation {
public:
  PlanarTransformation();
  ~PlanarTransformation();

  // rellative transformations
  void zoom(double zoom, const Eigen::Vector2d &image_size, bool save_history);
  void translate(const Eigen::Vector2d &translation,
                 const Eigen::Vector2d &image_size, bool save_history);

  void setRellative(double zoom, const Eigen::Vector2d &translation,
                    const Eigen::Vector2d &image_size, bool save_history);

  void setNewZoomWindowFromPicture(
      const geometry::Rect &zoom_window_picture_coordinates,
      const Eigen::Vector2d &image_size, bool save_history);

  void
  setNewZoomWindowFromWorld(const geometry::Rect &zoom_window_world_coordinates,
                            const Eigen::Vector2d &image_size,
                            bool save_history);

  void transformToWorld(const Eigen::Vector2d &picture,
                        Eigen::Vector2d &world) const;
  void transformToPicture(const Eigen::Vector2d &world,
                          Eigen::Vector2d &picture) const;

  void historyStepBack();

  void initHomography(const Eigen::Vector2d &image_size,
                      const geometry::Rect &world_corners);

  double getCurrentZoom() const;

private:
  void debugInformation(const Eigen::Vector2d &image_size);

  static void
  getPictureReferenceABCD(const Eigen::Vector2d &image_size,
                          std::array<Eigen::Vector2d, 4> &picturereference);

  static void
  getRectReferenceABCD(const geometry::Rect &rect,
                       std::array<Eigen::Vector2d, 4> &reference_points,
                       bool given_rect_from_picture);
  void saveCurrentToHistory();

  // clang-format off
  /*
   * isometry
   * f = "zoom"
   * x,y = translation
   *
   *      |f  0  x|
   * H =  |0  f  y|
   *      |0  0  1|
   * */
  //clang-format on
  Eigen::Matrix3d homographyWorld2Picture;
  Eigen::Matrix3d homographyPicture2World;

  typedef std::vector<Eigen::Matrix3d,  Eigen::aligned_allocator<Eigen::Matrix3d>> History;
  typedef History::iterator HistoryIt;

  History history;
  int history_current_index = -1;

};

}
#endif
