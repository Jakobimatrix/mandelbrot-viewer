#ifndef DISPALY_OPEN_CV_H
#define DISPLAY_OPEN_CV_H

#include <base/structs.hpp>
#include <display/display.h>
#include <eigen3/Eigen/Core>
#include <opencv2/opencv.hpp>

namespace disp {

constexpr char WINDOW_NAME[] = "OpenCV Window";

class DisplayOpenCV : public Display {
public:
  DisplayOpenCV();

  ~DisplayOpenCV();

  void changeResolution(int x, int y);

  void updateImage() override;

  void close() override;

  bool isRunning() override;

  bool setPixelColor(int x, int y, const color::RGB<int> &rgb) override;
  bool setPixelColor(int x, int y, const color::RGB<double> &rgb) override;
  bool setPixelColor(int x, int y, const color::HSV<int> &rgb) override;
  bool setPixelColor(int x, int y, const color::HSV<double> &rgb) override;

  int getWindowSizeX() const override;

  int getWindowSizeY() const override;

  Eigen::Vector2d imageSize() const override;

  void drawRect(const geometry::Rect &rect) override;

  void saveCurrentImage() const override;

  void drawNoUpdate() override;

  void renderVideo() override;

  static void callUserMouseInteractionCallback(int event, int x, int y,
                                               int flags, void *me);

  static void dbgSliderCallback(int, void *);

private:
  cv::Mat image;

  int dbg1 = static_cast<int>(0.2 * SLIDER_TICKS);
  int dbg2 = static_cast<int>(0.4 * SLIDER_TICKS);
  int dbg3 = static_cast<int>(0.5 * SLIDER_TICKS);
  int dbg4 = static_cast<int>(0.7 * SLIDER_TICKS);

  static const int SLIDER_TICKS = 1000;
};
} // namespace disp

#endif
