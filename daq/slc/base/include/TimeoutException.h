#ifndef _Belle2_TimeoutException_hh
#define _Belle2_TimeoutException_hh

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class TimeoutException : public IOException {

  public:
    TimeoutException() throw() {}
    TimeoutException(const std::string& comment, ...) throw();
    TimeoutException(int err, const std::string& comment, ...) throw();
    ~TimeoutException() throw() {}

  };

}

#endif
