#include "Exception.h"

#include <sstream>

using namespace Belle2;

Exception::Exception() throw()
  : _file_name(""), _line_no(0), _comment("")
{
}

Exception::Exception(const std::string& file_name,
                     int line,
                     const std::string& comment) throw()
  : _file_name(file_name), _line_no(line), _comment(comment)
{
}

Exception::~Exception() throw()
{

}


const char* Exception::what() const throw()
{
  static std::string str;
  std::stringstream ss;
  ss.clear();
  ss.str("");
  ss << "@File('" << _file_name
     << "',line=" << _line_no
     << "), comment='" << _comment
     << "'";
  str = ss.str();
  return str.c_str();

}
