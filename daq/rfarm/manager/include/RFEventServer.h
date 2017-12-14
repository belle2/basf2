#ifndef RFEVENTSERVER_H
#define RFEVENTSERVER_H
//+
// File : RFEventServer.h
// Description : Receive events from EVB1 and distribute them
//               to processing nodes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
//-

#include <string>
#include <sys/types.h>
#include <sys/wait.h>

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFSharedMem.h"
#include "daq/rfarm/manager/RFProcessManager.h"
#include "daq/rfarm/manager/RFLogManager.h"
#include "daq/rfarm/manager/RFNSM.h"
#include "daq/rfarm/manager/RFFlowStat.h"

#include "daq/rfarm/manager/RFServerBase.h"

#define MAXNODES 256

namespace Belle2 {

  class RFEventServer : public RFServerBase {
  public:
    RFEventServer(std::string conffile);
    ~RFEventServer();

    // Instance of singleton
    static RFEventServer& Create(std::string conffile);
    static RFEventServer& Instance();

    // Functions to be hooked to NSM
    int Configure(NSMmsg*, NSMcontext*);
    int UnConfigure(NSMmsg*, NSMcontext*);
    int Start(NSMmsg*, NSMcontext*);
    int Stop(NSMmsg*, NSMcontext*);
    int Restart(NSMmsg*, NSMcontext*);

    // Server function
    void server();

  private:
    RFConf*            m_conf;
    RFSharedMem*       m_shm;
    RFProcessManager*  m_proc;
    RFLogManager*      m_log;
    RFFlowStat*        m_flow;
    RingBuffer*        m_rbufin;

    int m_pid_recv;
    int m_pid_sender[MAXNODES];
    int m_nnodes;

  private:
    static RFEventServer* s_instance;
  };

}
#endif




