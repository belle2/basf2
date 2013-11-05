#ifndef _Belle2_TTDCallback_hh
#define _Belle2_TTDCallback_hh

#include "FTSWController.h"

#include <nsm/RCCallback.h>
#include <nsm/RunStatus.h>

#include <system/Fork.h>

#include <base/TTDNode.h>

namespace Belle2 {

  class RunStatus;
  class TTDData;

  class TTDCallback : public RCCallback {

  public:
    TTDCallback(TTDNode* node = NULL);
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
    TTDNode* _node;
    std::vector<FTSWController> _ftswcon_v;
    Fork _fork;

  };

}

#endif
