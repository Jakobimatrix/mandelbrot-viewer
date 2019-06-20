#ifndef MACROS_H
#define MACROS_H

#include <iostream>

// this flag is set by CMAKE depending weather
// -DCMAKE_BUILD_TYPE=Debug or =Release
#ifdef NDEBUG
#define DEBUGVAR(x)
#define DEBUGMSG(x)
#else
#define DEBUGVAR(x)                                                            \
  do {                                                                         \
    std::cerr << #x << ": " << std::endl << x << std::endl;                    \
  } while (0)

#define DEBUGMSG(x)                                                            \
  do {                                                                         \
    std::cerr << x << std::endl;                                               \
  } while (0)
#endif

#endif
