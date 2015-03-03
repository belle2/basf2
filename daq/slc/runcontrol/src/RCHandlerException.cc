#include "daq/slc/runcontrol/RCHandlerException.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

using namespace Belle2;

RCHandlerException::RCHandlerException(const std::string& format, ...) throw()
{
  va_list ap;
  char ss[1024 * 100];
  va_start(ap, format);
  vsprintf(ss, format.c_str(), ap);
  va_end(ap);
  m_comment = ss;
}

RCHandlerException::RCHandlerException(int err, const std::string& format, ...) throw()
{
  m_err = err;
  va_list ap;
  char ss[1024 * 100];
  va_start(ap, format);
  vsprintf(ss, format.c_str(), ap);
  va_end(ap);
  m_comment = ss;
}
