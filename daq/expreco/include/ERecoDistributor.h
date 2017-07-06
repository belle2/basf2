#ifndef EREVENTDIST_H
#define EREVENTDIST_H
//+
// File : ERecoDistributor.h
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

  class ERecoDistributor : public RFServerBase {
  public:
    ERecoDistributor(std::string conffile);
    ~ERecoDistributor();

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

    int m_pid_recv[MAXNODES];
    int m_pid_sender[MAXNODES];
    int m_nnodes;
    int m_nrecv;

  };

}
#endif




