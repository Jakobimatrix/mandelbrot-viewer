#ifndef DISPALY_H
#define DISPLAY_H

#include <base/color.hpp>
#include <base/planarTransformation.h>
#include <base/structs.hpp>
#include <boost/function.hpp>
#include <eigen3/Eigen/Core>
#include <mandelbrot/mandelbrot.h>
#include <thread>

namespace disp {

#ifdef NDEBUG
constexpr int DEFAULT_RESOLUTION_X = 1800;
constexpr int DEFAULT_RESOLUTION_Y = 1000;
#else
constexpr int DEFAULT_RESOLUTION_X = 300;
constexpr int DEFAULT_RESOLUTION_Y = 300;
#endif

typedef boost::function<color::RGB<int>(const Eigen::Vector2d &,
                                        const Eigen::Vector2i &, bool)>
    DrawPixelWiseRGB;
typedef boost::function<color::HSV<int>(const Eigen::Vector2d &,
                                        const Eigen::Vector2i &, bool)>
    DrawPixelWiseHSV;

class Display {
public:
  virtual void changeResolution(int x, int y) = 0;

  virtual bool isRunning() = 0;

  virtual void close() = 0;

  virtual bool setPixelColor(int x, int y, const color::RGB<int> &) = 0;

  virtual int getWindowSizeX() const = 0;

  virtual int getWindowSizeY() const = 0;

  virtual Eigen::Vector2d imageSize() const = 0;

  void setDrawFunction(const DrawPixelWiseRGB &);

  void setDrawFunction(const DrawPixelWiseHSV &);

  void setNumThreads(int num_threads);

  bool startUpdateLoop();

  void transformToProportionalRect(geometry::Rect &rect) const;

  enum DRAWING_FUNKTION {
    NOISE,
    TEST,
    MANDELBROT_GRAY,
    MANDELBROT_COLORED,
  };

  void setDrawFunction(DRAWING_FUNKTION df);

protected:
  Display();

  ~Display();

  enum EVENT {
    LEFT_MOUSE_UP,
    LEFT_MOUSE_DOWN,
    RIGHT_MOUSE_CLICK,
    MOUSE_MOVE,
    PICTURE,
    OTHER
  };

  virtual void updateImage() = 0;

  virtual void drawRect(const geometry::Rect &rect) = 0;

  virtual void drawNoUpdate() = 0;

  virtual void saveCurrentImage() const = 0;

  Eigen::Vector2d getCurrentWorldPosition() const;

  double getCurrentWorldZoom() const;

  std::string getCurrentPositionIdentifier() const;

  bool tempSetMandelbrotSpline(const EigenSTL::vector_Vector2d &splinePoints);

  bool tempSetDebugParam(double dbg1, double dbg2, double dbg3, double dbg4);

  void userMouseInteractionCallback(EVENT event,
                                    const Eigen::Vector2d &mousePos);

  color::HSV<int> convertPixelWiseRGB2HSV(const Eigen::Vector2d &position,
                                          const Eigen::Vector2i &save,
                                          bool load);

  color::RGB<int> convertPixelWiseHSV2RGB(const Eigen::Vector2d &position,
                                          const Eigen::Vector2i &save,
                                          bool load);

  // virtual void callUserMouseInteractionCallback() = 0; TODO

  int resolution_x = DEFAULT_RESOLUTION_X;
  int resolution_y = DEFAULT_RESOLUTION_Y;

  int num_threads = 1;

  DrawPixelWiseRGB drawPixelWiseRGB;
  DrawPixelWiseHSV drawPixelWiseHSV;

  std::thread *main_loop;
  bool main_loop_running = false;

  void setMandelbrotIterations(unsigned int iterations);

  unsigned int getMandelbrotIterations() const;

private:
  void threadedMainLoop();

  void calculateImageMultiThreaded(int from, int to, bool load_from_stored);

  void calculateImageSingleThreaded(bool load_from_stored);

  void calculateImage(bool load_from_stored);

  void userInteractions();

  void redrawLastFrame();

  const color::HSV<int> testbild(const Eigen::Vector2d &position,
                                 const Eigen::Vector2i &save, bool load);

  const color::RGB<int> noSignal(const Eigen::Vector2d &position,
                                 const Eigen::Vector2i &save, bool load);

  const color::HSV<int> mandelbrotColored(const Eigen::Vector2d &position,
                                          const Eigen::Vector2i &save,
                                          bool load);

  const color::RGB<int> mandelbrotGray(const Eigen::Vector2d &position,
                                       const Eigen::Vector2i &save, bool load);

  conv::PlanarTransformation planar_transformation;

  Eigen::Vector2d mouse_picture_corner1;
  Eigen::Vector2d mouse_picture_corner2;
  Eigen::Vector2d current_mouse_picture_pos;
  bool zoom = false;
  bool draw_zoom_window = false;
  bool need_update = true;

  Mandelbrot mandelbrot;

  Eigen::MatrixXi lastData;

  unsigned int mandelbrot_iterations = 2000;
};
} // namespace disp

#endif
