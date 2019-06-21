# mandelbrot-viewer
Simple tool to zoom around the mandelbrot set.

### Features so far:
 * Zoom into the mandelbrot set: Klick mouse button left and hold than move your mouse to define the next zoom window. Release mouse button left.
 * double klick mouse button right anywhere inside the window to zoom one step back (needs recalculation yet)
 * change the appearance of the mandelbrot set by draging the sliders on top.
 * Save a picture of the current window by double clicking the middle mouse button 
 
## Todos
 - [ ] Use Qt for controllers.
 - [ ] Control number of threads and resolution (and other options via gui)
 - [ ] Make it possible to save and revisit locations
 - [ ] Multithreading not optimal yet
 - [ ] Convert given "path of locations" to zoom movie
 
## Build the project (Linux)

 1. Install dependencies:
  * boost
  * open CV
  * Eigen
 2. clone mandelbrot-viewer
 3. Dont forget to update the submodules **git submodule update --init --recursive**
 4. Have a look at ''/mandelbrot-viewer/src/executables/src/main.cpp'' and set the numbers of threads suiting your hardware.
 5. Have a look at ''/mandelbrot-viewer/src/display/src/display/display.h'' and set the DEFAULT_RESOLUTION and maybe DEFAULT_MANDELBROT_ITERATIONS
 6. Go into ''/mandelbrot-viewer/Release'' and run the ./build.sh which does first run cmake and than buids the traget
 7. If build was successful use the ./run.sh to run the new builded executable
 8. If you want to tinker with the code you might want to build in Debug mode (same procedure inside the ''Debug'' folder.
 
 


