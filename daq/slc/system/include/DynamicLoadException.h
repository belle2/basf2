#ifndef _Belle2_DynamicLoadException_hh
#define _Belle2_DynamicLoadException_hh

#include "daq/slc/base/Exception.h"

namespace Belle2 {

  class DynamicLoadException : public Exception {

  public:
    DynamicLoadException(const std::string& format, ...) throw();

  };

}

#endif
