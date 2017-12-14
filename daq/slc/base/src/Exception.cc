#include "daq/slc/base/Exception.h"

#include <sstream>

using namespace Belle2;

Exception::Exception() throw()
  : m_comment(""), m_err(0)
{
}

Exception::Exception(const std::string& comment) throw()
  : m_comment(comment), m_err(0)
{
}

Exception::Exception(const std::string& comment,
                     int err) throw()
  : m_comment(comment), m_err(err)
{
}

Exception::~Exception() throw()
{

}


const char* Exception::what() const throw()
{
  return m_comment.c_str();
}
