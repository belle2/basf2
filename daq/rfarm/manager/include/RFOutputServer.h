#ifndef RFOUTPUTSERVER_H
#define RFOUTPUTSERVER_H
//+
// File : RFOutputServer.h
// Description : Collect outputs from worker nodes and send them
//               to EVB2 w/ branch to PXD
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
//-

#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFSharedMem.h"
#include "daq/rfarm/manager/RFProcessManager.h"
#include "daq/rfarm/manager/RFLogManager.h"
#include "daq/rfarm/manager/RFNSM.h"
#include "daq/rfarm/manager/RFFlowStat.h"

#include "daq/rfarm/manager/RFServerBase.h"

#define MAXNODES 256

namespace Belle2 {

  class RFOutputServer : public RFServerBase {
  public:
    RFOutputServer(std::string conffile);
    ~RFOutputServer();

    // Functions to be hooked to NSM
    int Configure(NSMmsg*, NSMcontext*);
    int UnConfigure(NSMmsg*, NSMcontext*);
    int Start(NSMmsg*, NSMcontext*);
    int Stop(NSMmsg*, NSMcontext*);
    int Restart(NSMmsg*, NSMcontext*);

    // Server function
    void server();

    // Cleanup
    void cleanup();

  private:
    RFConf*            m_conf;
    RFSharedMem*       m_shm;
    RFProcessManager*  m_proc;
    RFLogManager*      m_log;
    RFFlowStat*        m_flow;
    RingBuffer*        m_rbufin;
    RingBuffer*        m_rbufout;

    int m_pid_receiver[MAXNODES];
    int m_pid_basf2;
    int m_pid_sender;
    int m_pid_hrecv;
    int m_pid_hrelay;
    int m_nnodes;

  };

}
#endif




