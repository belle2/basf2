#ifndef _Belle2_EB1TXController_h
#define _Belle2_EB1TXController_h

#include "daq/slc/apps/rocontrold/ROController.h"

namespace Belle2 {

  class EB1TXController : public ROController {

  public:
    virtual ~EB1TXController() throw();

  protected:
    virtual void initArguments(const DBObject& obj);
    virtual bool loadArguments(const DBObject& obj);

  };

}

#endif
