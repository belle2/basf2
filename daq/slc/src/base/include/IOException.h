#ifndef _Belle2_IOException_hh
#define _Belle2_IOException_hh

#include "Exception.h"

namespace Belle2 {

  class IOException : public Exception {

  public:
    IOException(const std::string& file_name,
                const int line_no,
                const std::string& comment) throw()
      : Exception(file_name, line_no, comment) {}

  };

}

#endif
