#ifndef _Belle2_RCHandlerException_h
#define _Belle2_RCHandlerException_h

#include <daq/slc/nsm/NSMHandlerException.h>

namespace Belle2 {

  class RCHandlerException : public NSMHandlerException {

  public:
    RCHandlerException(const std::string& comment = "") throw();
    RCHandlerException(const char* comment, ...) throw();
    ~RCHandlerException() throw() {}

  };

}

#endif
