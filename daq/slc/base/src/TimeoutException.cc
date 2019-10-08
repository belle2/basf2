#include "daq/slc/base/TimeoutException.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

using namespace Belle2;

TimeoutException::TimeoutException(const std::string& format, ...)
{
  va_list ap;
  char ss[1024 * 100];
  va_start(ap, format);
  vsprintf(ss, format.c_str(), ap);
  va_end(ap);
  m_comment = ss;
}

TimeoutException::TimeoutException(int err, const std::string& format, ...)
{
  m_err = err;
  va_list ap;
  char ss[1024 * 100];
  va_start(ap, format);
  vsprintf(ss, format.c_str(), ap);
  va_end(ap);
  m_comment = ss;
}

