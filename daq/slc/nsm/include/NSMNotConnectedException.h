#ifndef _Belle2_NSMNotConnectedException_h
#define _Belle2_NSMNotConnectedException_h

#include "daq/slc/nsm/NSMHandlerException.h"

namespace Belle2 {

  class NSMNotConnectedException : public NSMHandlerException {

  public:
    NSMNotConnectedException(const std::string& format)
      : NSMHandlerException(format) {}

  };

}

#endif
