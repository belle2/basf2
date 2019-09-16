#ifndef _Belle2_NSMHandlerException_hh
#define _Belle2_NSMHandlerException_hh

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class NSMHandlerException : public IOException {

  public:
    NSMHandlerException(const std::string& format, ...);
    NSMHandlerException() {}
#if __GNUC__ >= 7
    ~NSMHandlerException() {}
#else
    ~NSMHandlerException() throw() {}
#endif

  };

}

#endif
