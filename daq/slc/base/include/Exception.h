#ifndef _Belle2_Exception_hh
#define _Belle2_Exception_hh

#include <exception>
#include <string>

namespace Belle2 {

  class Exception : public std::exception {

  private:
    std::string _file_name;
    int _line_no;
    std::string _comment;

  public:
    Exception() throw();
    Exception(const std::string&, const int,
              const std::string&) throw();
    virtual ~Exception() throw();

  public:
    virtual const char* what() const throw();

  };

}

#endif
