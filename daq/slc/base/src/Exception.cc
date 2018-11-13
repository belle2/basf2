#include "daq/slc/base/Exception.h"

#include <sstream>

using namespace Belle2;

Exception::Exception()
  : m_comment(""), m_err(0)
{
}

Exception::Exception(const std::string& comment)
  : m_comment(comment), m_err(0)
{
}

Exception::Exception(const std::string& comment,
                     int err)
  : m_comment(comment), m_err(err)
{
}

#if __GNUC__ >= 7
const char* Exception::what() const noexcept
#else
const char* Exception::what() const throw()
#endif
{
  return m_comment.c_str();
}
