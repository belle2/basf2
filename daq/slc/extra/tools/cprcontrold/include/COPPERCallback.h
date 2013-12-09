#ifndef _Belle2_COPPERCallback_hh
#define _Belle2_COPPERCallback_hh

#include "daq/slc/base/NSMNode.h"

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/PThread.h"

#include "daq/slc/readout/RunInfoBuffer.h"
#include "daq/slc/readout/RunLogMessanger.h"

#include "daq/slc/nsm/RCCallback.h"

#include "HSLBController.h"

namespace Belle2 {

  class ProcessListener;

  class COPPERCallback : public RCCallback {

  public:
    COPPERCallback(NSMNode* node = NULL);
    virtual ~COPPERCallback() throw();

  public:
    virtual void init() throw();
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
    std::string _buf_path;
    std::string _fifo_path;
    RunInfoBuffer _buf;
    RunLogMessanger _msg;
    HSLBController _hslbcon_v[4];
    Fork _fork;
    PThread _thread;
    ProcessListener* _listener;
    int _confno;

  };

}

#endif
