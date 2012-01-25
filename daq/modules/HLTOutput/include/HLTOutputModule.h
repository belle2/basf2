#ifndef HLTOUTPUTMODULE_H
#define HLTOUTPUTMODULE_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>
#include <framework/pcore/RingBuffer.h>

namespace Belle2 {
  class HLTOutputModule : public Module {
  public:
    HLTOutputModule();
    virtual ~HLTOutputModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    void test(unsigned int times);
    void sendTerminate();

  private:
    RingBuffer* m_buffer;

    int m_times;
  };
}

#endif
