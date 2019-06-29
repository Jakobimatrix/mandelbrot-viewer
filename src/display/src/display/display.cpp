#include <base/conversations.h>
#include <base/functions.hpp>
#include <base/macros.hpp>
#include <base/randomGenerators.h>
#include <boost/bind.hpp>
#include <display/display.h>
//#include <time.h>

#include <vector>

namespace disp {

Display::Display() {
  Eigen::Vector2d imgSize(DEFAULT_RESOLUTION_X, DEFAULT_RESOLUTION_Y);

  // Zoom the world to have (-2,2) matching top left image corner and (2,-2)
  // bottom-right corner.

  const double window_proportion = static_cast<double>(DEFAULT_RESOLUTION_X) /
                                   static_cast<double>(DEFAULT_RESOLUTION_Y);
  const Eigen::Vector2d top_left(0., 0.);
  const Eigen::Vector2d bottom_right(4. * window_proportion, -4.);
  geometry::Rect initial_zoom(top_left, bottom_right);
  initial_zoom.setCenter(Eigen::Vector2d(0, 0));

  planar_transformation.initHomography(imgSize, initial_zoom);

  mandelbrot.setMaxIterations(mandelbrot_iterations);

  mandelbrot.setSmoothing(true);

  lastData.resize(DEFAULT_RESOLUTION_X, DEFAULT_RESOLUTION_Y);
}

Display::~Display() {}

void Display::transformToProportionalRect(geometry::Rect &rect) const {
  const double window_proportion = static_cast<double>(getWindowSizeY()) /
                                   static_cast<double>(getWindowSizeX());
  const double proportional_height = rect.width() * window_proportion;

  const Eigen::Vector2d center = rect.center();

  rect.corner2.y() = rect.corner1.y() + proportional_height;

  rect.setCenter(center);
}

void Display::setDrawFunction(const DrawPixelWiseRGB &function) {
  drawPixelWiseRGB = function;
  drawPixelWiseHSV =
      boost::bind(&Display::convertPixelWiseRGB2HSV, this, _1, _2, _3);
}

void Display::setDrawFunction(const DrawPixelWiseHSV &function) {
  drawPixelWiseHSV = function;
  drawPixelWiseRGB =
      boost::bind(&Display::convertPixelWiseHSV2RGB, this, _1, _2, _3);
}

color::HSV<int>
Display::convertPixelWiseRGB2HSV(const Eigen::Vector2d &position,
                                 const Eigen::Vector2i &save, bool load) {
  const color::RGB<int> rgb = drawPixelWiseRGB(position, save, load);
  const color::HSV<int> hsv = color::convertToHSV(rgb);
  return hsv;
}

color::RGB<int>
Display::convertPixelWiseHSV2RGB(const Eigen::Vector2d &position,
                                 const Eigen::Vector2i &save, bool load) {
  const color::HSV<int> hsv = drawPixelWiseHSV(position, save, load);
  const color::RGB<int> rgb = color::convertToRGB(hsv);
  return rgb;
}

void Display::calculateImageMultiThreaded(bool load_from_stored) {
  const int size_vertical = getWindowSizeY();
  Eigen::Vector2d mandelbrotCoordinates;
  int from, to;

  while (multithreadManager.getNextPackage(from, to)) {
    for (int i = from; i < to; i++) {
      const int x = i / size_vertical;
      const int y = i % size_vertical;
      const Eigen::Vector2d imageCoordinates(x, y);

      planar_transformation.transformToWorld(imageCoordinates,
                                             mandelbrotCoordinates);

      setPixelColor(x, y,
                    drawPixelWiseRGB(mandelbrotCoordinates,
                                     Eigen::Vector2i(x, y), load_from_stored));
    }
  }
}

void Display::calculateImageSingleThreaded(bool load_from_stored) {
  Eigen::Vector2d mandelbrotCoordinates;
  for (int x = 0; x < getWindowSizeX(); x++) {
    for (int y = 0; y < getWindowSizeY(); y++) {
      const Eigen::Vector2d imageCoordinates(x, y);
      planar_transformation.transformToWorld(imageCoordinates,
                                             mandelbrotCoordinates);
      setPixelColor(x, y,
                    drawPixelWiseRGB(mandelbrotCoordinates,
                                     Eigen::Vector2i(x, y), load_from_stored));
    }
    // updateImage(); slows everything down
  }
}

void Display::calculateImage(bool load_from_stored) {
  if (num_threads > 1) {
    const int numPixel = getWindowSizeX() * getWindowSizeY();
    // Dont make the size too small to avoid "too much access to the Thread
    // manager which is mutexed. Dont make the size too big to avoid one thread
    // to be finnished with nothing to do left while others are still
    // calculateing.
    const int package_size = getWindowSizeX();
    multithreadManager.reset(package_size, numPixel);

    std::vector<std::thread> threadpool;

    for (int t = 0; t < num_threads; t++) {
      // starting the thread
      threadpool.push_back(std::thread(&Display::calculateImageMultiThreaded,
                                       this, load_from_stored));
    }

    // wait until all are finnished
    std::for_each(threadpool.begin(), threadpool.end(),
                  std::mem_fn(&std::thread::join));

  } else {
    calculateImageSingleThreaded(load_from_stored);
  }
}

void Display::setNumThreads(int num_threads_) { num_threads = num_threads_; }

bool Display::startUpdateLoop() {
  if (main_loop_running) {
    return false;
  }
  main_loop_running = true;
  main_loop = new std::thread(&Display::threadedMainLoop, this);
  return true;
}

void Display::threadedMainLoop() {
  while (isRunning()) {
    if (need_update) {
      timer.start();
      calculateImage(false);
      timer.stop();
      std::cout << timer << std::endl;
      need_update = false;
      updateImage();
    } else {
      userInteractions();
    }
  }
}

void Display::userInteractions() {
  if (zoom) {
    zoom = false;

    // get the rect the user has drawn
    geometry::Rect zoom_frame(mouse_picture_corner1, mouse_picture_corner2);

    // stay proportional
    transformToProportionalRect(zoom_frame);

    // set new zoom
    planar_transformation.setNewZoomWindowFromPicture(zoom_frame, imageSize(),
                                                      true);
    need_update = true;

  } else if (draw_zoom_window) {
    geometry::Rect zoom_frame(mouse_picture_corner1, current_mouse_picture_pos);
    // stay proportional
    transformToProportionalRect(zoom_frame);
    drawRect(zoom_frame);
  } else {
    drawNoUpdate();
  }
}

void Display::userMouseInteractionCallback(EVENT event,
                                           const Eigen::Vector2d &mousePos) {
  // for drawing the zoom rectangle
  if (event == EVENT::LEFT_MOUSE_DOWN) {
    mouse_picture_corner1 = mousePos;
    draw_zoom_window = true;
  } else if (event == EVENT::LEFT_MOUSE_UP) {
    mouse_picture_corner2 = mousePos;
    zoom = true;
    draw_zoom_window = false;
  } else if (event == EVENT::MOUSE_MOVE) {
    current_mouse_picture_pos = mousePos;
  } else if (event == EVENT::RIGHT_MOUSE_CLICK) {
    mandelbrot.setSmoothing(!mandelbrot.getSmoothing());
    // planar_transformation.historyStepBack();
    need_update = true;
  } else if (event == EVENT::PICTURE) {
    saveCurrentImage();
  }
}

Eigen::Vector2d Display::getCurrentWorldPosition() const {
  Eigen::Vector2d world;
  planar_transformation.transformToWorld(imageSize() * 0.5, world);
  return world;
}

double Display::getCurrentWorldZoom() const {
  return planar_transformation.getCurrentZoom();
}

std::string Display::getCurrentPositionIdentifier() const {
  const Eigen::Vector2d worldCenter = getCurrentWorldPosition();
  return "MandelBrot X_" + std::to_string(worldCenter.x()) + " Y_" +
         std::to_string(worldCenter.y()) + " F_" +
         std::to_string(getCurrentWorldZoom());
}

void Display::setDrawFunction(DRAWING_FUNKTION df) {

  switch (df) {
  case DRAWING_FUNKTION::NOISE: {
    const disp::DrawPixelWiseRGB f =
        boost::bind<color::RGB<int>>(&Display::noSignal, this, _1, _2, _3);
    setDrawFunction(f);
    break;
  }
  case DRAWING_FUNKTION::TEST: {
    const disp::DrawPixelWiseHSV f =
        boost::bind<color::HSV<int>>(&Display::testbild, this, _1, _2, _3);
    setDrawFunction(f);
    break;
  }
  case DRAWING_FUNKTION::MANDELBROT_GRAY: {
    const disp::DrawPixelWiseRGB f = boost::bind<color::RGB<int>>(
        &Display::mandelbrotGray, this, _1, _2, _3);
    setDrawFunction(f);
    break;
  }
  case DRAWING_FUNKTION::MANDELBROT_COLORED: {
    const disp::DrawPixelWiseHSV f = boost::bind<color::HSV<int>>(
        &Display::mandelbrotColored, this, _1, _2, _3);
    setDrawFunction(f);
    break;
  }
  }
}

const color::HSV<int> Display::testbild(const Eigen::Vector2d &position,
                                        const Eigen::Vector2i &save,
                                        bool load) {
  double h_deg;
  if (load) {
    h_deg = lastData(save.x(), save.y());
  } else {
    h_deg = 100. * std::abs(position.x()) + 100. * std::abs(position.y());
    lastData(save.x(), save.y()) = h_deg;
  }
  color::HSV<int> result;
  mandelbrot.setMaxIterations(255);
  mandelbrot.mandelbrotHSV(func::hsvNormalize255(h_deg), result);
  return result;
}

const color::RGB<int> Display::noSignal(const Eigen::Vector2d &position,
                                        const Eigen::Vector2i &save,
                                        bool load) {
  const int rgb = func::uniform_int_dist(0, 255);
  return color::RGB<int>(rgb, rgb, rgb);
}

const color::HSV<int>
Display::mandelbrotColored(const Eigen::Vector2d &position,
                           const Eigen::Vector2i &save, bool load) {

  double iterations;
  if (load) {
    iterations = lastData(save.x(), save.y());
  } else {
    iterations = mandelbrot.mandelbrot(position);
    lastData(save.x(), save.y()) = iterations;
  }

  color::HSV<int> hsv;
  mandelbrot.mandelbrotHSV(iterations, hsv);
  return hsv;
}

const color::RGB<int> Display::mandelbrotGray(const Eigen::Vector2d &position,
                                              const Eigen::Vector2i &save,
                                              bool load) {

  double iterations;
  if (load) {
    iterations = lastData(save.x(), save.y());
  } else {
    iterations = mandelbrot.mandelbrot(position);
    lastData(save.x(), save.y()) = iterations;
  }

  color::RGB<int> rgb;
  mandelbrot.mandelbrotGreyScale(iterations, rgb);
  return rgb;
}

bool Display::tempSetDebugParam(double dbg1, double dbg2, double dbg3,
                                double dbg4) {
  EigenSTL::vector_Vector2d splinePoints;

  splinePoints.push_back(Eigen::Vector2d(0, 0));
  splinePoints.push_back(Eigen::Vector2d(dbg1, dbg2));
  splinePoints.push_back(Eigen::Vector2d(dbg3, dbg4));
  splinePoints.push_back(
      Eigen::Vector2d(mandelbrot_iterations, mandelbrot_iterations));

  tempSetMandelbrotSpline(splinePoints);
}
bool Display::tempSetMandelbrotSpline(
    const EigenSTL::vector_Vector2d &splinePoints) {
  mandelbrot.setSpline(splinePoints);
  redrawLastFrame();
}

void Display::redrawLastFrame() {
  // we dont calculate stuff here, so singlethread is probably faster
  calculateImageSingleThreaded(true);
  updateImage();
}

void Display::setMandelbrotIterations(unsigned int iterations) {
  mandelbrot_iterations = iterations;

  mandelbrot.setMaxIterations(iterations);
  // todo reset spline
  calculateImage(false);
  updateImage();
}
unsigned int Display::getMandelbrotIterations() const {
  return mandelbrot_iterations;
}

void Display::MultithreadManager::reset(int package_size_, int max_index_) {
  current_managed_index = 0;
  packet_size = package_size_;
  max_index = max_index_;
}

bool Display::MultithreadManager::getNextPackage(int &from, int &to) {
  access_thread_manager.lock();
  bool work_not_done = true;
  from = current_managed_index;
  to = from + packet_size;
  if (to > max_index) {
    to = max_index;
    if (from >= max_index) {
      work_not_done = false;
    }
  }
  current_managed_index = to;
  access_thread_manager.unlock();
  return work_not_done;
}

} // namespace disp
