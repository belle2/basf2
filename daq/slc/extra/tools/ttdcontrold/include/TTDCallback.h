#ifndef _Belle2_TTDCallback_hh
#define _Belle2_TTDCallback_hh

#include "FTSWController.h"

#include <daq/slc/nsm/RCCallback.h>
#include <daq/slc/nsm/RunStatus.h>

#include <daq/slc/system/Fork.h>

#include <daq/slc/base/NSMNode.h>

namespace Belle2 {

  class TTDCallback : public RCCallback {

  public:
    TTDCallback(NSMNode* node);
    virtual ~TTDCallback() throw();

  public:
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool abort() throw();

  private:
    std::vector<FTSWController> _ftswcon_v;
    Fork _fork;

  };

}

#endif
