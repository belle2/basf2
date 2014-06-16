#ifndef _Belle2_DBHandlerException_hh
#define _Belle2_DBHandlerException_hh

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class DBHandlerException : public IOException {

  public:
    DBHandlerException(const std::string& format, ...) throw();

  };

}

#endif
