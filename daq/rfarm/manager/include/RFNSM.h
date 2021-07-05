/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef RFNSM_H
#define RFNSM_H
//
// State definitions
//
// State transition of RFARM
//      "configure"    "start"   "pause"
//  OFFLINE<->CONFIGURED<->RUNNING<->PAUSE
//    ^ "unconfigure"            "resume"
//    +---------"restart"
//
#define RFSTATE_UNCONFIGURED 0
#define RFSTATE_CONFIGURED 1
#define RFSTATE_TRANSITION 2
#define RFSTATE_RUNNING 3
#define RFSTATE_IDLE 4

#include <string>

#include "nsm2/nsm2.h"

#include "daq/rfarm/manager/RFServerBase.h"
#include "daq/rfarm/manager/RfNodeInfo.h"

static std::string RFSTATE[] = { "Unconfigured", "Configured", "Transition", "Running", "Idle" };

namespace Belle2 {

  // Class to handle global reference to status
  class RFNSM_Status {
  public:
    RFNSM_Status();
    ~RFNSM_Status();

    static RFNSM_Status& Instance();

    void set_flag(int);
    int get_flag();

    void set_state(int);
    int get_state();

  private:
    int m_flag;
    int m_state;
    static RFNSM_Status* s_instance;
  };

  // Class to interface to NSM
  class RFNSM {
  public:
    RFNSM(char* nodename, RFServerBase*);
    ~RFNSM();

    void AllocMem(char* format);
    RfNodeInfo* GetNodeInfo();

    static NSMcontext* GetContext() { return g_context; }

  private:
    std::string m_nodename;
    std::string m_formatfile;
    RfNodeInfo* m_info;
    static NSMcontext* g_context;
    //    RFServerBase* m_server;

    // Wrappers to be called from NSM
    static void m_Configure(NSMmsg*, NSMcontext*);
    static void m_UnConfigure(NSMmsg*, NSMcontext*);
    static void m_Start(NSMmsg*, NSMcontext*);
    static void m_Stop(NSMmsg*, NSMcontext*);
    static void m_Pause(NSMmsg*, NSMcontext*);
    static void m_Resume(NSMmsg*, NSMcontext*);
    static void m_Restart(NSMmsg*, NSMcontext*);
    static void m_Status(NSMmsg*, NSMcontext*);

    // Functions to receive response from function call
    static void m_OK(NSMmsg*, NSMcontext*);
    static void m_ERROR(NSMmsg*, NSMcontext*);

  };
};
#endif




