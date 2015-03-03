#ifndef _Belle2_EB0Controller_h
#define _Belle2_EB0Controller_h

#include "daq/slc/apps/rocontrold/ROController.h"

namespace Belle2 {

  class EB0Controller : public ROController {

  public:
    virtual ~EB0Controller() throw();

  protected:
    virtual bool initArguments(const DBObject& obj) throw();
    virtual void loadArguments() throw();

  };

}

#endif
