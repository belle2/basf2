#ifndef _Belle2_DBHandlerException_hh
#define _Belle2_DBHandlerException_hh

#include "base/IOException.h"

namespace Belle2 {

  class DBHandlerException : public IOException {

  public:
    DBHandlerException(const std::string& file_name,
                       const int line_no,
                       const std::string& comment) throw()
      : IOException(file_name, line_no, comment) {}

  };

}

#endif
