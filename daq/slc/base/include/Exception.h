/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_Exception_hh
#define _Belle2_Exception_hh

#include <exception>
#include <string>

namespace Belle2 {

  class Exception : public std::exception {

  public:
    Exception();
    Exception(const std::string&);
    Exception(const std::string&, int err);
#if __GNUC__ >= 7
    virtual ~Exception() {}
#else
    virtual ~Exception() throw() {}
#endif

  public:
#if __GNUC__ >= 7
    virtual const char* what() const noexcept;
#else
    virtual const char* what() const throw();
#endif
    int err() const { return m_err; }

  protected:
    std::string m_comment;
    int m_err;

  };

}

#endif
