#include <array>
#include <base/macros.hpp>
#include <base/planarTransformation.h>
#include <base/typedefs.hpp>
#include <eigen3/Eigen/Geometry>
#include <homography/homography.hpp>

namespace conv {

PlanarTransformation::PlanarTransformation() {
  homographyWorld2Picture = Eigen::Matrix3d::Identity();
  homographyWorld2Picture(1, 1) =
      -1; // y in picture is -y in reight handed coordinate system
  homographyPicture2World = homographyWorld2Picture.inverse();
  setZerosInHomogen(homographyPicture2World);
}

void PlanarTransformation::getPictureReferenceABCD(
    const Eigen::Vector2d &image_size,
    std::array<Eigen::Vector2d, 4> &picturereference) {
  picturereference[0] << 0, 0;              // A
  picturereference[1] << image_size.x(), 0; // B
  picturereference[2] << image_size;        // C
  picturereference[3] << 0, image_size.y(); // D
}

void PlanarTransformation::getRectReferenceABCD(
    const geometry::Rect &rect,
    std::array<Eigen::Vector2d, 4> &reference_points,
    bool given_rect_from_picture) {
  // make sure the points are in the correct order
  const geometry::Rect clean = rect;

  if (given_rect_from_picture) {
    reference_points[0] = clean.corner1;                         // A
    reference_points[1] << clean.corner2.x(), clean.corner1.y(); // B
    reference_points[2] = clean.corner2;                         // C
    reference_points[3] << clean.corner1.x(), clean.corner2.y(); // D
  } else {
    reference_points[1] = clean.corner1;                         // B
    reference_points[2] << clean.corner2.x(), clean.corner1.y(); // C
    reference_points[3] = clean.corner2;                         // D
    reference_points[0] << clean.corner1.x(), clean.corner2.y(); // A
  }
}

void PlanarTransformation::initHomography(const Eigen::Vector2d &image_size,
                                          const geometry::Rect &world_corners) {
  // set correspondence 1:1
  homographyWorld2Picture = Eigen::Matrix3d::Identity();
  homographyWorld2Picture(1, 1) =
      -1; // y in picture is -y in reight handed coordinate system
  homographyPicture2World = homographyWorld2Picture.inverse();
  setZerosInHomogen(homographyPicture2World);
  setNewZoomWindowFromWorld(world_corners, image_size, true);
}

PlanarTransformation::~PlanarTransformation() {}

void PlanarTransformation::zoom(double zoom, const Eigen::Vector2d &image_size,
                                bool save_history) {
  const Eigen::Vector2d new_image_frame = image_size * zoom;
  const Eigen::Vector2d translate = (image_size - new_image_frame) * 0.5;
  // Set up a window with the size and position of the window and zoom in.
  const geometry::Rect rect(translate, new_image_frame);
  // Use setNewZoomWindowFromPicture to calculate new homography for translated
  // window.
  setNewZoomWindowFromPicture(rect, image_size, save_history);
}

void PlanarTransformation::translate(const Eigen::Vector2d &translation,
                                     const Eigen::Vector2d &image_size,
                                     bool save_history) {

  // Set up a window with the size and position of the window and translate it.
  const geometry::Rect rect(translation, image_size + translation);
  // Use setNewZoomWindowFromPicture to calculate new homography for translated
  // window.
  setNewZoomWindowFromPicture(rect, image_size, save_history);
}

void PlanarTransformation::setRellative(double zoom_,
                                        const Eigen::Vector2d &translation,
                                        const Eigen::Vector2d &image_size,
                                        bool save_history) {
  zoom(zoom_, image_size, false);
  translate(translation, image_size, save_history);
}

void PlanarTransformation::setNewZoomWindowFromPicture(
    const geometry::Rect &zoom_window_picture_coordinates,
    const Eigen::Vector2d &image_size, bool save_history) {

  // get the 4 corners of the window
  std::array<Eigen::Vector2d, 4> picture_reference;
  getPictureReferenceABCD(image_size, picture_reference);

  // get the 4 corners of the zoom rectangle
  std::array<Eigen::Vector2d, 4> zoom_reference;
  getRectReferenceABCD(zoom_window_picture_coordinates, zoom_reference, true);

  std::array<EigenSTL::pair_Vector2d_Vector2d, 4> associated_points;

  // get the corresponding world point of the zoom rect useing current
  // homography
  for (unsigned int i = 0; i < 4; i++) {
    Eigen::Vector2d world;
    transformToWorld(zoom_reference[i], world);

    // set the 4 corners of the window as assosiated points to the calculated
    // world point
    associated_points[i] = std::make_pair(world, picture_reference[i]);
  }

  // find the new homography
  homographyWorld2Picture = tool::Homography::findHomography(
      associated_points, tool::Homography::Methode::PARTIAL_PIV_LU);
  setZerosInHomogen(homographyWorld2Picture);
  homographyPicture2World = homographyWorld2Picture.inverse();
  setZerosInHomogen(homographyPicture2World);
  if (save_history) {
    saveCurrentToHistory();
  }
}

void PlanarTransformation::setNewZoomWindowFromWorld(
    const geometry::Rect &zoom_window_world_coordinates,
    const Eigen::Vector2d &image_size, bool save_history) {

  // transform the given rect to picture coordinates useing current homography
  geometry::Rect tempRect;
  transformToPicture(zoom_window_world_coordinates.corner1, tempRect.corner1);
  transformToPicture(zoom_window_world_coordinates.corner2, tempRect.corner2);

  // use setNewZoomWindowFromPicture to calculate new homography.
  setNewZoomWindowFromPicture(tempRect, image_size, save_history);
}

void PlanarTransformation::transformToWorld(const Eigen::Vector2d &picture,
                                            Eigen::Vector2d &world) const {
  world =
      tool::Homography::computeTransformation(homographyPicture2World, picture);
}

void PlanarTransformation::transformToPicture(const Eigen::Vector2d &world,
                                              Eigen::Vector2d &picture) const {
  picture =
      tool::Homography::computeTransformation(homographyWorld2Picture, world);
}

double PlanarTransformation::getCurrentZoom() const {
  return homographyWorld2Picture(1, 1);
}

void PlanarTransformation::recordCurrentPerspective() {
  recordedPerspective.push_back(history[history_current_index]);
  std::cout << "recorded current viewe" << std::endl;
}

double PlanarTransformation::createPlayback() {
  std::vector<double> X, Y, F, T;
  double t = -1;
  for (const auto &h : recordedPerspective) {
    t++;
    F.push_back(h(0, 0));
    X.push_back(h(0, 2));
    Y.push_back(h(1, 2));
    T.push_back(t);
    std::cout << t << ": " << h(0, 0) << std::endl;
  }
  recorded_zoom.set_boundary(tk::spline::second_deriv, 0.0,
                             tk::spline::first_deriv, 0.0, false);
  recorded_zoom.set_points(T, F, false);
  recorded_x.set_boundary(tk::spline::second_deriv, 0.0,
                          tk::spline::first_deriv, 0.0, false);
  recorded_x.set_points(T, X, false);
  recorded_y.set_boundary(tk::spline::second_deriv, 0.0,
                          tk::spline::first_deriv, 0.0, false);
  recorded_y.set_points(T, Y, false);
  recorded_time_end = t;

  for (double i = 0; i < t; i = i + 0.01) {
    std::cout << i << ": " << recorded_zoom(i) << std::endl;
  }
  return t;
}

bool PlanarTransformation::setWindow2RecordedTime(double t) {
  if (t > recorded_time_end) {
    return false;
  }
  setZerosInHomogen(homographyWorld2Picture);
  homographyWorld2Picture(0, 0) = recorded_zoom(t);
  homographyWorld2Picture(0, 2) = recorded_x(t);
  homographyWorld2Picture(1, 1) = -recorded_zoom(t);
  homographyWorld2Picture(1, 2) = recorded_y(t);
  homographyPicture2World = homographyWorld2Picture.inverse();
  setZerosInHomogen(homographyPicture2World);
  return true;
}

void PlanarTransformation::clearRecord() {
  recordedPerspective.clear();
  recorded_time_end = -1;
}

void PlanarTransformation::saveCurrentToHistory() {
  history.push_back(homographyWorld2Picture);
  history_current_index = history.size() - 1;
}

void PlanarTransformation::historyStepBack() {
  if (history_current_index < 1) {
    return;
  }
  history_current_index--;
  homographyWorld2Picture = history[history_current_index];
  homographyPicture2World = homographyWorld2Picture.inverse();
  setZerosInHomogen(homographyPicture2World);
  history.pop_back();
}

void PlanarTransformation::setZerosInHomogen(Eigen::Matrix3d &h) const {
  h(0, 1) = 0;
  h(1, 0) = 0;
  h(2, 0) = 0;
  h(2, 1) = 0;
  h(2, 2) = 1;
}

void PlanarTransformation::debugInformation(const Eigen::Vector2d &image_size) {
  std::array<Eigen::Vector2d, 4> picture_reference;
  getPictureReferenceABCD(image_size, picture_reference);
  for (unsigned int i = 0; i < 4; i++) {
    Eigen::Vector2d world;
    transformToWorld(picture_reference[i], world);
  }
}
} // namespace conv
