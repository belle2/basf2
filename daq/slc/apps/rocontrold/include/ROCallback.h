#ifndef _Belle2_ROCallback_hh
#define _Belle2_ROCallback_hh

#include "daq/slc/nsm/RCCallback.h"

#include "daq/slc/base/NSMNode.h"

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/PThread.h"

#include "daq/slc/readout/RunInfoBuffer.h"
#include "daq/slc/readout/RunLogMessanger.h"

namespace Belle2 {

  class ROCallback : public RCCallback {

  public:
    ROCallback(NSMNode* node);
    virtual ~ROCallback() throw();

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
    std::string _buf_path;
    std::string _fifo_path;
    RunInfoBuffer _buf;
    RunLogMessanger _msg;
    Fork _fork;
    PThread _thread;

  };

}

#endif
