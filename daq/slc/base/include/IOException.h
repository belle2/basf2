#ifndef _Belle2_IOException_hh
#define _Belle2_IOException_hh

#include "daq/slc/base/Exception.h"

namespace Belle2 {

  class IOException : public Exception {

  public:
    IOException() {}
    IOException(const std::string& comment, ...);
    IOException(int err, const std::string& comment, ...);
#if __GNUC__ >= 7
    virtual ~IOException() {}
#else
    virtual ~IOException() throw() {}
#endif

  };

}

#endif
