#ifndef HLTINPUTMODULE_H
#define HLTINPUTMODULE_H

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
  class HLTInputModule : public Module {
  public:
    HLTInputModule();
    virtual ~HLTInputModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    EHLTStatus getData();
    void writeFile(char* data, int size);

  private:
    std::string m_nodeType;

    RingBuffer* m_buffer;
    MsgHandler* m_msgHandler;

    int m_eventsTaken;
    int m_nDataSources;
  };
}

#endif
