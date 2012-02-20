#ifndef HLTOUTPUTMODULE_H
#define HLTOUTPUTMODULE_H

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
  class HLTOutputModule : public Module {
  public:
    HLTOutputModule();
    virtual ~HLTOutputModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    void putData(const DataStore::EDurability& durability);
    EHLTStatus testData(char* buffer);
    void sendTerminate();

    bool checkData(std::string data1, char* data2);
    void writeFile(char* data, int size);

  private:
    RingBuffer* m_buffer;
    MsgHandler* m_msgHandler;

    std::string m_nodeType;
    int m_eventsSent;

    std::vector<std::string> m_branchNames[DataStore::c_NDurabilityTypes];
    bool m_done[DataStore::c_NDurabilityTypes];
    StoreIter* m_objectIterator[DataStore::c_NDurabilityTypes];
    StoreIter* m_arrayIterator[DataStore::c_NDurabilityTypes];
  };
}

#endif
