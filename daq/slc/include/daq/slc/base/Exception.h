#ifndef _Belle2_Exception_hh
#define _Belle2_Exception_hh

#include <exception>
#include <string>

namespace Belle2 {

  class Exception : public std::exception {

  public:
    Exception() throw();
    Exception(const std::string&) throw();
    virtual ~Exception() throw();

  public:
    virtual const char* what() const throw();

  protected:
    std::string m_comment;

  };

}

#endif
