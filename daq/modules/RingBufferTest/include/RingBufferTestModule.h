#ifndef RINGBUFFERTESTMODULE_H
#define RINGBUFFERTESTMODULE_H

#include <string>
#include <fstream>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>
#include <framework/pcore/RingBuffer.h>

namespace Belle2 {
  class RingBufferTestModule : public Module {
  public:
    RingBufferTestModule();
    virtual ~RingBufferTestModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:
    RingBuffer* m_buffer;
    RingBuffer* m_buffer2;
    RingBuffer* m_buffer3;
    std::fstream m_output;
    bool m_flag;
  };
}

#endif
