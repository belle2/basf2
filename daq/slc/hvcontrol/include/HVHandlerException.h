#ifndef _Belle2_HVHandlerException_h
#define _Belle2_HVHandlerException_h

#include <daq/slc/nsm/NSMHandlerException.h>

namespace Belle2 {

  class HVHandlerException : public NSMHandlerException {

  public:
    HVHandlerException(const std::string& comment, ...) throw();
    HVHandlerException(int err, const std::string& comment, ...) throw();
    ~HVHandlerException() throw() {}

  };

}

#endif
