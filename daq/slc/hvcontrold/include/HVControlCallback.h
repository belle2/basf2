#ifndef _Belle2_HVControlCallback_hh
#define _Belle2_HVControlCallback_hh

#include "HVCallback.h"

#include <vector>

namespace Belle2 {

  class HVControlCallback : public HVCallback {

  public:
    HVControlCallback(NSMNode* node) throw();
    virtual ~HVControlCallback() throw() {}

  public:
    virtual void init() throw() {}
    virtual bool ok() throw();
    virtual bool error() throw();
    virtual bool load() throw();
    virtual bool switchOn() throw();
    virtual bool switchOff() throw();

  };

};

#endif
