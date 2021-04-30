#include "daq/slc/system/DynamicLoadException.h"

#include <cstdarg>
#include <cstdio>

using namespace Belle2;

DynamicLoadException::DynamicLoadException(const std::string& format, ...)
{
  va_list ap;
  char ss[1024 * 100];
  va_start(ap, format);
  vsnprintf(ss, sizeof(ss), format.c_str(), ap);
  va_end(ap);
  m_comment = ss;
}

