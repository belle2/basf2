#ifndef _Belle2_NSMHandlerException_hh
#define _Belle2_NSMHandlerException_hh

#include "base/IOException.h"

namespace Belle2 {

  class NSMHandlerException : public IOException {

  public:
    NSMHandlerException(const std::string& file_name,
                        const int line_no,
                        const std::string& comment) throw()
      : IOException(file_name, line_no, comment) {}

  };

}

#endif
