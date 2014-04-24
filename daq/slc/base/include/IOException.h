#ifndef _Belle2_IOException_hh
#define _Belle2_IOException_hh

#include "daq/slc/base/Exception.h"

namespace Belle2 {

  class IOException : public Exception {

  public:
    IOException() throw() {}
    IOException(const std::string& comment, ...) throw();
    ~IOException() throw() {}

  };

}

#endif
