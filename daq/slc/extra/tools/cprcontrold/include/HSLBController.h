#ifndef _Belle2_HSLBController_h
#define _Belle2_HSLBController_h

#include <daq/slc/database/ConfigObject.h>

#include <mgt/mgt.h>

namespace Belle2 {

  class HSLBController {

  public:
    HSLBController();
    ~HSLBController() throw();

  public:
    bool reset() throw();
    bool load(const ConfigObject& obj) throw();

  private:
    Mgt_t* m_mgt;

  };

}

#endif
