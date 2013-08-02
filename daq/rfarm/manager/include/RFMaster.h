#ifndef RF_MASTER_H
#define RF_MASTER_H
//+
// File : RFMaster.h
// Description : Master node for RFARM
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 31 - Jul - 2013
//-

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFSharedMem.h"
#include "daq/rfarm/manager/RFProcessManager.h"
#include "daq/rfarm/manager/RFLogManager.h"
#include "daq/rfarm/manager/RFNSM.h"

#include "daq/rfarm/manager/RFServerBase.h"

namespace Belle2 {
  class RFMaster : public RFServerBase {
  public:
    RFMaster(std::string conf);
    ~RFMaster();

    // Functions to be hooked to NSM
    void Configure(NSMmsg*, NSMcontext*);
    void Start(NSMmsg*, NSMcontext*);
    void Stop(NSMmsg*, NSMcontext*);
    void Restart(NSMmsg*, NSMcontext*);

    // Server function
    void monitor_loop();

  private:
    RFConf*            m_conf;
    RFSharedMem*       m_shm;
    RFLogManager*      m_log;

  };
}
#endif



