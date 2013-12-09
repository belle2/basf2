#ifndef _Belle2_DynamicLoadException_hh
#define _Belle2_DynamicLoadException_hh

#include "daq/slc/base/Exception.h"

namespace Belle2 {

  class DynamicLoadException : public Exception {

  public:
    DynamicLoadException(const std::string& file_name,
                         const int line_no,
                         const std::string& comment) throw()
      : Exception(file_name, line_no, comment) {}

  };

}

#endif
