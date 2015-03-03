#ifndef _Belle2_Stream0Controller_h
#define _Belle2_Stream0Controller_h

#include "daq/slc/apps/rocontrold/ROController.h"

namespace Belle2 {

  class Stream0Controller : public ROController {

  public:
    virtual ~Stream0Controller() throw();

  public:
    void check() throw();

  protected:
    virtual bool initArguments(const DBObject& obj) throw();
    virtual void loadArguments() throw();

  };

}

#endif
