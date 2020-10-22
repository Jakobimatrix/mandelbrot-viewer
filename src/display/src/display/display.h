#ifndef DISPALY_H
#define DISPALY_H

#include <base/color.hpp>
#include <base/conversations.h>
#include <base/functions.hpp>
#include <base/macros.hpp>
#include <base/planarTransformation.h>
#include <base/randomGenerators.h>
#include <base/structs.hpp>
#include <mandelbrot/mandelbrot.h>
#include <timer/timer.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <eigen3/Eigen/Core>
#include <mutex>
#include <thread>
#include <vector>

namespace disp {

#ifdef NDEBUG
constexpr int DEFAULT_RESOLUTION_X = 1800;
constexpr int DEFAULT_RESOLUTION_Y = 850;
#else
constexpr int DEFAULT_RESOLUTION_X = 600;
constexpr int DEFAULT_RESOLUTION_Y = 600;
#endif

typedef boost::function<void(const int, const int)> DrawPixelWise;

struct MultithreadManager {
  std::mutex access_thread_manager;
  int packet_size;
  int current_managed_index;
  int max_index;

  void reset(int package_size_, int max_index_) {
    current_managed_index = 0;
    packet_size = package_size_;
    max_index = max_index_;
  }

  bool getNextPackage(int &from, int &to) {
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
};

class Display {
public:
  enum COLORING { COS, SPLINE };

  virtual bool isRunning() = 0;

  virtual void close() = 0;

  virtual bool setPixelColor(int x, int y, const color::RGB<int> &) = 0;
  virtual bool setPixelColor(int x, int y, const color::RGB<double> &) = 0;
  virtual bool setPixelColor(int x, int y, const color::HSV<int> &) = 0;
  virtual bool setPixelColor(int x, int y, const color::HSV<double> &) = 0;

  virtual int getWindowSizeX() const = 0;

  virtual int getWindowSizeY() const = 0;

  virtual Eigen::Vector2d imageSize() const = 0;

  void setDrawFunction(COLORING coloring) {
    switch (coloring) {
    case COLORING::COS: {
      normalise_mandelbrot_iterations = true;
      drawPixelWise_f = boost::bind(&Display::drawMandelbrotCOS, this, _1, _2);
      break;
    }
    case COLORING::SPLINE: {
      normalise_mandelbrot_iterations = true;
      drawPixelWise_f =
          boost::bind(&Display::drawMandelbrotSPLINE, this, _1, _2);
      break;
    }
    }
  }

  void setNumThreads(int num_threads_) { num_threads = num_threads_; }

  bool startUpdateLoop() {
    if (main_loop_running) {
      return false;
    }
    main_loop_running = true;
    main_loop = new std::thread(&Display::threadedMainLoop, this);
    return true;
  }

  void transformToProportionalRect(geometry::Rect &rect) const {
    const double window_proportion = static_cast<double>(getWindowSizeY()) /
                                     static_cast<double>(getWindowSizeX());
    const double proportional_height = rect.width() * window_proportion;
    const Eigen::Vector2d center = rect.center();
    rect.corner2.y() = rect.corner1.y() + proportional_height;
    rect.setCenter(center);
  }

protected:
  Display() {
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
    mandelbrot.setSmoothing(true);

    setDrawFunction(COLORING::COS);

    lastData.resize(DEFAULT_RESOLUTION_X, DEFAULT_RESOLUTION_Y);
  }

  ~Display() {}

  enum EVENT {
    LEFT_MOUSE_UP,
    LEFT_MOUSE_DOWN,
    RIGHT_MOUSE_CLICK,
    MOUSE_MOVE,
    PICTURE,
    RECORD,
    RENDER,
    OTHER
  };

  virtual void updateImage() = 0;

  virtual void drawRect(const geometry::Rect &rect) = 0;

  virtual void drawNoUpdate() = 0;

  virtual void saveCurrentImage() const = 0;

  virtual void renderVideo() = 0;

  Eigen::Vector2d getCurrentWorldPosition() const {
    Eigen::Vector2d world;
    planar_transformation.transformToWorld(imageSize() * 0.5, world);
    return world;
  }

  double getCurrentWorldZoom() const {
    return planar_transformation.getCurrentZoom();
  }

  std::string getCurrentPositionIdentifier() const {
    const Eigen::Vector2d worldCenter = getCurrentWorldPosition();
    return "MandelBrot X_" + std::to_string(worldCenter.x()) + " Y_" +
           std::to_string(worldCenter.y()) + " F_" +
           std::to_string(getCurrentWorldZoom());
  }

  // set debug params between 0 and 1
  bool tempSetDebugParam(double dbg1, double dbg2, double dbg3, double dbg4) {

    // setMandelbrotIterations(dbg1 * 1000);
    // std::cout << "iterate " << dbg1 * 1000 << std::endl;
    EigenSTL::vector_Vector2d splinePoints;

    splinePoints.push_back(Eigen::Vector2d(0, 0));
    splinePoints.push_back(Eigen::Vector2d(dbg1, dbg2) *
                           mandelbrot.getMaxIterations());
    splinePoints.push_back(Eigen::Vector2d(dbg3, dbg4) *
                           mandelbrot.getMaxIterations());
    splinePoints.push_back(Eigen::Vector2d(mandelbrot.getMaxIterations(),
                                           mandelbrot.getMaxIterations()));

    mandelbrot.setSpline(splinePoints);
    mandelbrot.setCosParams(dbg1 * 1, dbg2 * M_PI_2, dbg3 * M_PI_2,
                            dbg4 * M_PI_2);
    need_update = true;
  }

  void userMouseInteractionCallback(EVENT event,
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
      planar_transformation.historyStepBack();
      need_update = true;
    } else if (event == EVENT::PICTURE) {
      saveCurrentImage();
    } else if (event == EVENT::RECORD) {
      planar_transformation.recordCurrentPerspective();
    } else if (event == EVENT::RENDER) {
      renderVideo();
    }
  }

  // virtual void callUserMouseInteractionCallback() = 0; TODO

  unsigned int getMandelbrotIterations() const {
    return mandelbrot.getMaxIterations();
  }

  double createPlayback() { return planar_transformation.createPlayback(); }

  bool setWindow2RecordedTime(double t) {
    if (planar_transformation.setWindow2RecordedTime(t)) {
      need_update = true;
      return true;
    }
    return false;
  }

  void calculateImage(bool load_from_stored) {
    chooseNumCalculations();
    if (load_from_stored) {
      drawAllPixel();
      return;
    }
    if (num_threads > 1) {
      const int numPixel = getWindowSizeX() * getWindowSizeY();
      // Dont make the size too small to avoid "too much access to the Thread
      // manager which is mutexed. Dont make the size too big to avoid one
      // thread to be finnished with nothing to do left while others are still
      // calculateing.
      const int package_size = getWindowSizeX();
      multithreadManager.reset(package_size, numPixel);

      std::vector<std::thread> threadpool;

      for (int t = 0; t < num_threads; t++) {
        // starting the thread
        threadpool.push_back(
            std::thread(&Display::calculateImageMultiThreaded, this));
      }

      // wait until all are finnished
      std::for_each(threadpool.begin(), threadpool.end(),
                    std::mem_fn(&std::thread::join));

    } else {
      calculateImageSingleThreaded();
    }

    if (normalise_mandelbrot_iterations) {
      normalizeLastData();
    }

    drawAllPixel();
  }

private:
  void chooseNumCalculations() {
    // iteration_resolution low: 100, high: 1000
    const double iteration_resolution = 1000;
    const double max_log_zoom = 35;
    // depending on zoom factor wee need more iterations
    const double zoom = std::log(-getCurrentWorldZoom());
    const unsigned int iterations =
        static_cast<unsigned int>(zoom * iteration_resolution / max_log_zoom) +
        62;
    mandelbrot.setMaxIterations(iterations);
    std::cout << "zoom: " << zoom << "-"
              << "iterations: " << iterations << std::endl;
  };

  void drawAllPixel() {
    for (int col = 0; col < resolution_x; col++) {
      for (int row = 0; row < resolution_y; row++) {
        drawPixelWise_f(col, row);
      }
    }
  }

  void threadedMainLoop() {
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

  void calculateImageMultiThreaded() {
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
        const int iterations = mandelbrot.mandelbrot(mandelbrotCoordinates);
        lastData(x, y) = iterations;
      }
    }
  }

  void calculateImageSingleThreaded() {
    Eigen::Vector2d mandelbrotCoordinates;
    for (int x = 0; x < DEFAULT_RESOLUTION_X; x++) {
      for (int y = 0; y < DEFAULT_RESOLUTION_Y; y++) {
        const Eigen::Vector2d imageCoordinates(x, y);
        planar_transformation.transformToWorld(imageCoordinates,
                                               mandelbrotCoordinates);

        const int iterations = mandelbrot.mandelbrot(mandelbrotCoordinates);
        lastData(x, y) = iterations;
      }
    }
  }

  void drawMandelbrotCOS(int x, int y) {
    const color::RGB<double> rgb = mandelbrot.mandelbrotCOS(lastData(x, y));
    // function provided by child class
    setPixelColor(x, y, rgb);
  }

  void drawMandelbrotSPLINE(int x, int y) {
    const color::HSV<double> hsv = mandelbrot.mandelbrotSPLINE(lastData(x, y));
    // function provided by child class
    setPixelColor(x, y, hsv);
  }

  void userInteractions() {
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
      geometry::Rect zoom_frame(mouse_picture_corner1,
                                current_mouse_picture_pos);
      // stay proportional
      transformToProportionalRect(zoom_frame);
      drawRect(zoom_frame);
    } else {
      drawNoUpdate();
    }
  }

  // only calculate the colors, not the mandelbrotiterations
  void redrawLastFrame() {
    drawAllPixel();
    updateImage();
  }

  void normalizeLastData() {
    const Eigen::VectorXd max_row_values = lastData.rowwise().maxCoeff();
    const Eigen::VectorXd min_row_values = lastData.rowwise().minCoeff();
    double max = 0;
    double min = mandelbrot.getMaxIterations();
    for (int i = 0; i < resolution_x; i++) {
      if (max < max_row_values(i)) {
        max = max_row_values(i);
      }
      if (min > min_row_values(i)) {
        min = min_row_values(i);
      }
    }
    const double span = max - min;
    const double multiply =
        static_cast<double>(mandelbrot.getMaxIterations()) / span;
    lastData = (lastData.array() - min) * multiply;
  }

  conv::PlanarTransformation planar_transformation;
  Eigen::Vector2d mouse_picture_corner1;
  Eigen::Vector2d mouse_picture_corner2;
  Eigen::Vector2d current_mouse_picture_pos;
  bool zoom = false;
  bool draw_zoom_window = false;
  bool need_update = true;
  Mandelbrot mandelbrot;
  Eigen::MatrixXd lastData;
  MultithreadManager multithreadManager;
  tool::Timer timer;
  bool normalise_mandelbrot_iterations = true;
  COLORING coloring = COLORING::SPLINE;
  DrawPixelWise drawPixelWise_f;

protected:
  int resolution_x = DEFAULT_RESOLUTION_X;
  int resolution_y = DEFAULT_RESOLUTION_Y;
  int num_threads = 1;
  std::thread *main_loop;
  bool main_loop_running = false;
};
} // namespace disp

#endif
