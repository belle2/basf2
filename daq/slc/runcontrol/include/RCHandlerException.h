#ifndef _Belle2_RCHandlerException_h
#define _Belle2_RCHandlerException_h

#include <daq/slc/nsm/NSMHandlerException.h>

namespace Belle2 {

  class RCHandlerException : public NSMHandlerException {

  public:
    RCHandlerException(const std::string& comment = "");
    RCHandlerException(const char* comment, ...);
#if __GNUC__ >= 7
    virtual ~RCHandlerException() {}
#else
    virtual ~RCHandlerException() throw() {}
#endif

  };

}

#endif
