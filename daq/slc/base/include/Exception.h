#ifndef _Belle2_Exception_hh
#define _Belle2_Exception_hh

#include <exception>
#include <string>

namespace Belle2 {

  class Exception : public std::exception {

  public:
    Exception() throw();
    Exception(const std::string&) throw();
    Exception(const std::string&, int err) throw();
    virtual ~Exception() throw();

  public:
    virtual const char* what() const throw();
    int err() const throw() { return m_err; }

  protected:
    std::string m_comment;
    int m_err;

  };

}

#endif
