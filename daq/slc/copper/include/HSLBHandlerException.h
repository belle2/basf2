#ifndef _Belle2_HSLBHandlerException_h
#define _Belle2_HSLBHandlerException_h

#include <daq/slc/nsm/NSMHandlerException.h>

namespace Belle2 {

  class HSLBHandlerException : public NSMHandlerException {

  public:
    HSLBHandlerException(const std::string& comment, ...) throw();
    HSLBHandlerException(int err, const std::string& comment, ...) throw();
    ~HSLBHandlerException() throw() {}

  };

}

#endif
