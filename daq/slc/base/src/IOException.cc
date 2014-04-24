#include "daq/slc/base/IOException.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

using namespace Belle2;

IOException::IOException(const std::string& format, ...) throw()
{
  va_list ap;
  char ss[1024 * 100];
  va_start(ap, format);
  vsprintf(ss, format.c_str(), ap);
  va_end(ap);
  _comment = ss;
}

