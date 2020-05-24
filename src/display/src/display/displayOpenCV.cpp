#include <base/macros.hpp>
#include <display/displayOpenCV.h>

namespace disp {

DisplayOpenCV::DisplayOpenCV()
    : image(DEFAULT_RESOLUTION_Y, DEFAULT_RESOLUTION_X, CV_8UC3,
            cv::Scalar(0, 0, 255)) {
  cv::imshow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);
  cv::setMouseCallback(disp::WINDOW_NAME,
                       DisplayOpenCV::callUserMouseInteractionCallback, this);

  char TrackbarName1[50];
  sprintf(TrackbarName1, "x1 %d", 1000);
  cv::createTrackbar(TrackbarName1, disp::WINDOW_NAME, &dbg1, SLIDER_TICKS,
                     DisplayOpenCV::dbgSliderCallback, this);

  char TrackbarName2[50];
  sprintf(TrackbarName2, "y1 %d", 1000);
  cv::createTrackbar(TrackbarName2, disp::WINDOW_NAME, &dbg2, SLIDER_TICKS,
                     DisplayOpenCV::dbgSliderCallback, this);

  char TrackbarName3[50];
  sprintf(TrackbarName3, "x2 x %d", 1000);
  cv::createTrackbar(TrackbarName3, disp::WINDOW_NAME, &dbg3, SLIDER_TICKS,
                     DisplayOpenCV::dbgSliderCallback, this);

  char TrackbarName4[50];
  sprintf(TrackbarName4, "y2 x %d", 1000);
  cv::createTrackbar(TrackbarName4, disp::WINDOW_NAME, &dbg4, SLIDER_TICKS,
                     DisplayOpenCV::dbgSliderCallback, this);
}

DisplayOpenCV::~DisplayOpenCV() {
  cv::setMouseCallback(disp::WINDOW_NAME, NULL, 0);
  close();
}

void DisplayOpenCV::close() { cv::destroyWindow(WINDOW_NAME); }

bool DisplayOpenCV::isRunning() {
  const bool window_is_open = cv::getWindowProperty(WINDOW_NAME, 0) >= 0;
  return window_is_open;
}

void DisplayOpenCV::changeResolution(int x, int y) {
  const cv::Rect oldViewe(0, 0, image.cols, image.rows);
  const cv::Rect newViewe(0, 0, x, y);
  const cv::Rect overlap = oldViewe & newViewe;

  cv::Mat newImage(x, y, CV_8UC3, cv::Scalar(0, 0, 255));
  const cv::Mat overlappingInfo = image(overlap).clone();

  overlappingInfo.copyTo(newImage);
  image = newImage;
}

int DisplayOpenCV::getWindowSizeX() const { return image.cols; }

int DisplayOpenCV::getWindowSizeY() const { return image.rows; }

Eigen::Vector2d DisplayOpenCV::imageSize() const {
  return Eigen::Vector2d(image.cols, image.rows);
}

bool DisplayOpenCV::setPixelColor(int x, int y, const color::RGB<int> &rgbi) {
  if (x > image.cols || y > image.rows) {
    return false;
  }
  // bgr!

  image.at<cv::Vec3b>(y, x) = cv::Vec3b(static_cast<unsigned char>(rgbi.b),
                                        static_cast<unsigned char>(rgbi.g),
                                        static_cast<unsigned char>(rgbi.r));
  return true;
}

bool DisplayOpenCV::setPixelColor(int x, int y, const color::RGB<double> &rgb) {
  const color::RGB<int> rgbi = rgb;
  return setPixelColor(x, y, rgbi);
}

bool DisplayOpenCV::setPixelColor(int x, int y, const color::HSV<double> &hsv) {
  const color::RGB<int> rgbi = hsv;
  return setPixelColor(x, y, rgbi);
}

bool DisplayOpenCV::setPixelColor(int x, int y, const color::HSV<int> &hsvi) {
  const color::RGB<int> rgbi = hsvi;
  return setPixelColor(x, y, rgbi);
}

void DisplayOpenCV::updateImage() {
  if (isRunning()) {
    cv::imshow(WINDOW_NAME, image);
    cv::waitKey(1);
  }
}

void DisplayOpenCV::drawRect(const geometry::Rect &rect) {
  auto copy = image.clone();
  if (isRunning()) {
    cv::rectangle(copy, cv::Point(rect.corner1.x(), rect.corner1.y()),
                  cv::Point(rect.corner2.x(), rect.corner2.y()),
                  cv::Scalar(42, 42, 255), 2);
    cv::imshow(WINDOW_NAME, copy);
    cv::waitKey(20);
  }
}

void DisplayOpenCV::drawNoUpdate() { cv::waitKey(20); }

void DisplayOpenCV::saveCurrentImage() const {
  const std::string name = getCurrentPositionIdentifier();
  const std::string path = "../images/";
  cv::imwrite(path + name + ".png", image);
  std::cout << "saved image to " << path + name + ".png" << std::endl;
}

void DisplayOpenCV::callUserMouseInteractionCallback(int event, int x, int y,
                                                     int flags, void *me) {
  // https://vovkos.github.io/doxyrest-showcase/opencv/sphinx_rtd_theme/enum_cv_MouseEventTypes.html
  EVENT own_event = EVENT::OTHER;
  // std::cout << "EVENT: " << event << std::endl;
  if (event == cv::EVENT_MBUTTONUP) {
    own_event = EVENT::PICTURE;
  } else if (event == cv::EVENT_LBUTTONDOWN) {
    own_event = EVENT::LEFT_MOUSE_DOWN;
  } else if (event == cv::EVENT_LBUTTONUP) {
    own_event = EVENT::LEFT_MOUSE_UP;
  } else if (event == cv::EVENT_MOUSEMOVE) {
    own_event = EVENT::MOUSE_MOVE;
  } else if (event == cv::EVENT_RBUTTONDOWN) {
    own_event = EVENT::RIGHT_MOUSE_CLICK;
  }
  const Eigen::Vector2d pos(x, y);
  static_cast<DisplayOpenCV *>(me)->userMouseInteractionCallback(own_event,
                                                                 pos);
}

void DisplayOpenCV::dbgSliderCallback(int i, void *me) {
  auto that = static_cast<DisplayOpenCV *>(me);
  const auto fn = [&](int val) {
    return static_cast<double>(val) / static_cast<double>(SLIDER_TICKS);
  };

  that->tempSetDebugParam(fn(that->dbg1), fn(that->dbg2), fn(that->dbg3),
                          fn(that->dbg4));
}

} // namespace disp
