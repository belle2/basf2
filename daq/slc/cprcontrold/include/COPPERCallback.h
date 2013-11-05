#ifndef _Belle2_COPPERCallback_hh
#define _Belle2_COPPERCallback_hh

#include "base/NSMNode.h"

#include "system/Fork.h"
#include "system/PThread.h"

#include "nsm/RCCallback.h"

#include "HSLBController.h"

namespace Belle2 {

  class ProcessListener;

  class COPPERCallback : public RCCallback {

  public:
    COPPERCallback(NSMNode* node = NULL);
    virtual ~COPPERCallback() throw();

  public:
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool abort() throw();

  private:
    int* openBuffer(size_t count, const char* path) throw();

  private:
    NSMNode* _node;
    int* _buf_config;
    int* _buf_status;
    HSLBController _hslbcon_v[4];
    Fork _fork;
    PThread _thread;
    ProcessListener* _listener;

  };

}

#endif
