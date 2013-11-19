#ifndef _Belle2_ArichHVCallback_hh
#define _Belle2_ArichHVCallback_hh

#include "HVCallback.h"

#include <vector>

namespace Belle2 {

  class ArichHVCallback : public HVCallback {

  public:
    ArichHVCallback(NSMNode* node) throw();
    virtual ~ArichHVCallback() throw() {}

  public:
    virtual bool load() throw();
    virtual bool switchOn() throw();
    virtual bool switchOff() throw();

  };

};

#endif
