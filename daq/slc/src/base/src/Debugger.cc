#include "Debugger.h"

#include <cstdarg>
#include <cstdio>
#include <iostream>

namespace Belle2 {

  void debug(const char* msg, ...)
  {
    va_list ap;
    char ss[1024];
    va_start(ap, msg);
    vsprintf(ss, msg, ap);
    va_end(ap);
    std::cout << ss << std::endl;
  }

}

