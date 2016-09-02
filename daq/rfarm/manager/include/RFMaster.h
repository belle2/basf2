#ifndef RF_MASTER_H
#define RF_MASTER_H
//+
// File : RFMaster.h
// Description : Master node for RFARM
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 31 - Jul - 2013
//-

#include <sys/types.h>
#include <sys/wait.h>

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
    int Configure(NSMmsg*, NSMcontext*);
    int UnConfigure(NSMmsg*, NSMcontext*);
    int Start(NSMmsg*, NSMcontext*);
    int Stop(NSMmsg*, NSMcontext*);
    int Restart(NSMmsg*, NSMcontext*);

    // Function to hook message funnctions
    void Hook_Message_Handlers();

    // Message functions
    static void Log_Handler(NSMmsg* msg, NSMcontext* ctx);

    // Server function
    void monitor_loop();


  private:
    RFConf*            m_conf;
    RFSharedMem*       m_shm;
    RFLogManager*      m_log;

  };

}
#endif



