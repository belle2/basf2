#ifndef _Belle2_RCTestModule_h
#define _Belle2_RCTestModule_h

#include <string>

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawCOPPER.h>

#include <daq/slc/readout/ProcessStatusBuffer.h>

namespace Belle2 {

  class RCTestModuleModule : public Module {

  public:

    RCTestModuleModule();
    virtual ~RCTestModuleModule();

    virtual void initialize();

    virtual void terminate();

    virtual void event();

  private:
    int m_id;
    std::string m_name;
    ProcessStatusBuffer m_buf;
    bool m_running;

  };

}

#endif
