#ifndef _Belle2_TimeoutException_hh
#define _Belle2_TimeoutException_hh

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class TimeoutException : public IOException {

  public:
    TimeoutException() {}
    TimeoutException(const std::string& comment, ...);
    TimeoutException(int err, const std::string& comment, ...);
#if __GNUC__ >= 7
    ~TimeoutException() {}
#else
    ~TimeoutException() throw() {}
#endif

  };

}

#endif
