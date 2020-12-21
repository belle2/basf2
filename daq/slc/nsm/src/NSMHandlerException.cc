#include "daq/slc/nsm/NSMHandlerException.h"

#include <cstdarg>
#include <cstdio>

using namespace Belle2;

NSMHandlerException::NSMHandlerException(const std::string& format, ...)
{
  va_list ap;
  char ss[1024 * 100];
  va_start(ap, format);
  vsnprintf(ss, sizeof(ss), format.c_str(), ap);
  va_end(ap);
  m_comment = ss;
}

