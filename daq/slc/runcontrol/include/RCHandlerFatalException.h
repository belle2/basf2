#ifndef _Belle2_RCHandlerFatalException_h
#define _Belle2_RCHandlerFatalException_h

#include <daq/slc/runcontrol/RCHandlerException.h>

namespace Belle2 {

  class RCHandlerFatalException : public RCHandlerException {

  public:
    RCHandlerFatalException(const std::string& comment = "") throw();
    RCHandlerFatalException(const char* comment, ...) throw();
    ~RCHandlerFatalException() throw() {}

  };

}

#endif
