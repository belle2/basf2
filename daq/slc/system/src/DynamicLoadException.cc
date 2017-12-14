#include "daq/slc/system/DynamicLoadException.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

using namespace Belle2;

DynamicLoadException::DynamicLoadException(const std::string& format, ...) throw()
{
  va_list ap;
  char ss[1024 * 100];
  va_start(ap, format);
  vsprintf(ss, format.c_str(), ap);
  va_end(ap);
  m_comment = ss;
}

