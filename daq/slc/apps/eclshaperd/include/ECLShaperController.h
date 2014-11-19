#ifndef Belle2_ECLShaperController_h
#define Belle2_ECLShaperController_h

#include "daq/slc/apps/eclshaperd/ECLShaper.h"
#include "daq/slc/apps/eclshaperd/ECLShaperConfig.h"

#include <daq/slc/base/IOException.h>

#include <string>
#include <vector>

namespace Belle2 {

  class ECLShaperController {

  public:
    ECLShaperController() {}
    ~ECLShaperController() {}

  public:
    bool boot(ECLShaperConfig& config) throw(IOException);
    bool init(ECLShaperConfig& config, int mode) throw(IOException);

  public:
    //bool status(int status_type = 0);

  private:
    std::vector<ECLShaper> m_shaper_v;

  };

}

#endif
