#ifndef _Belle2_ArichHVCallback_hh
#define _Belle2_ArichHVCallback_hh

#include "daq/slc/apps/hvcontrold/HVCallback.h"

#include <vector>

namespace Belle2 {

  class ArichHVCommunicator;

  class ArichHVCallback : public HVCallback {

  public:
    ArichHVCallback(NSMNode* node) throw();
    virtual ~ArichHVCallback() throw() {}

  public:
    virtual bool load() throw();
    virtual bool switchOn() throw();
    virtual bool switchOff() throw();

    void setHVCommunicator(ArichHVCommunicator* hv_comm) {
      _hv_comm = hv_comm;
    }

  private:
    ArichHVCommunicator* _hv_comm;

  };

};

#endif
