#ifndef ER_MASTER_H
#define ER_MASTER_H
//+
// File : ERecoMaster.h
// Description : Master node for Express Reco
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
  class ERecoMaster : public RFServerBase {
  public:
    ERecoMaster(std::string conf);
    ~ERecoMaster();

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



