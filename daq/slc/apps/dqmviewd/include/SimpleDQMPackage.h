#ifndef _Belle2_SimpleDQMPackage_h
#define _Belle2_SimpleDQMPackage_h

#include "daq/slc/dqm/DQMPackage.h"

namespace Belle2 {

  class SimpleDQMPackage : public DQMPackage {

  public:
    SimpleDQMPackage(const std::string& name,
                     const std::string& filename);
    virtual ~SimpleDQMPackage() throw();

  public:
    virtual void init();
    virtual void update();

  };

}

#endif
