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

#include <daq/slc/readout/RunInfoBuffer.h>
#include <daq/slc/readout/RunLogMessanger.h>

namespace Belle2 {

  class RCTestModuleModule : public Module {

  public:

    RCTestModuleModule();
    virtual ~RCTestModuleModule();

    virtual void initialize();

    virtual void terminate();

    virtual void event();

  private:
    std::string m_name;
    std::string m_buf_path;
    std::string m_msg_path;
    RunInfoBuffer m_buf;
    RunLogMessanger m_msg;

  };

}

#endif
