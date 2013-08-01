#ifndef RFNSM_H
#define RFNSM_H
//+
// File : RFNSM.h
// Description : NSM interface class for RFARM
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - June - 2013
//-
//
// State definitions
//
// State transition of RFARM
//      "configure"    "start"   "pause"
//  OFFLINE<->CONFIGURED<->RUNNING<->PAUSE
//    ^                         "resume"
//    +---------"restart"
//
#define RFSTATE_OFFLINE 0
#define RFSTATE_TRANS_CONFIG 1
#define RFSTATE_CONFIGURED 2
#define RFSTATE_TRANS_RUN 3
#define RFSTATE_RUNNING 4
#define RFSTATE_PAUSE 5


#include <string>
#include <stdio.h>
#include <unistd.h>

#include "nsm2/nsm2.h"
#include "nsm2/belle2nsm.h"

#include "daq/rfarm/manager/RFServerBase.h"
#include "daq/rfarm/manager/RfNodeInfo.h"

namespace Belle2 {

  // Class to interface to NSM

  class RFNSM {
  public:
    RFNSM(char* nodename, RFServerBase*);
    ~RFNSM();

    void AllocMem(char* format);
    RfNodeInfo* GetNodeInfo();

  private:
    std::string m_nodename;
    std::string m_formatfile;
    RfNodeInfo* m_info;
    //    RFServerBase* m_server;

    // Wrappers to be called from NSM
    static void m_Configure(NSMmsg*, NSMcontext*);
    static void m_Start(NSMmsg*, NSMcontext*);
    static void m_Stop(NSMmsg*, NSMcontext*);
    static void m_Pause(NSMmsg*, NSMcontext*);
    static void m_Resume(NSMmsg*, NSMcontext*);
    static void m_Restart(NSMmsg*, NSMcontext*);
    static void m_Status(NSMmsg*, NSMcontext*);

  };
};
#endif




