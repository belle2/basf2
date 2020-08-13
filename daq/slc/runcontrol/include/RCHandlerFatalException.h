#ifndef _Belle2_RCHandlerFatalException_h
#define _Belle2_RCHandlerFatalException_h

#include <daq/slc/runcontrol/RCHandlerException.h>

namespace Belle2 {

  class RCHandlerFatalException : public RCHandlerException {

  public:
    RCHandlerFatalException(const std::string& comment = "");
    RCHandlerFatalException(const char* comment, ...);
#if __GNUC__ >= 7
    virtual ~RCHandlerFatalException() {}
#else
    virtual ~RCHandlerFatalException() throw() {}
#endif

  };

}

#endif
