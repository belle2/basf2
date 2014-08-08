#ifndef _Belle2_DBObjectLoader_h
#define _Belle2_DBObjectLoader_h

#include <daq/slc/database/ConfigObject.h>

namespace Belle2 {

  class DBObjectLoader {

  public:
    static ConfigObject load(const std::string& path,
                             const std::string& tablename,
                             bool search = false);

  };

}

#endif
