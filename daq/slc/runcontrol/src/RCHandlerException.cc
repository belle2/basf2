#include "daq/slc/runcontrol/RCHandlerException.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

using namespace Belle2;

RCHandlerException::RCHandlerException(const std::string& comment)
{
  m_comment = comment;
}

RCHandlerException::RCHandlerException(const char* format, ...)
{
  va_list ap;
  char ss[1024 * 100];
  va_start(ap, format);
  vsprintf(ss, format, ap);
  va_end(ap);
  m_comment = ss;
}

