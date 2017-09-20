#ifndef EREVENTPROCESSOR_H
#define EREVENTPROCESSOR_H
//+
// File : ERecoEventProcessor.h
// Description : receive event, process it by basf2, and send output
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

#include "framework/pcore/RingBuffer.h"

#include "daq/rfarm/manager/RFServerBase.h"

#define MAXNODES 256

namespace Belle2 {

  class ERecoEventProcessor : public RFServerBase {
  public:
    ERecoEventProcessor(std::string conffile);
    ~ERecoEventProcessor();

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
    RingBuffer*        m_rbufin;
    RingBuffer*        m_rbufout;
    RFFlowStat*        m_flow;

    int m_pid_receiver;
    int m_pid_basf2;
    int m_pid_sender;
    int m_pid_hrecv;
    int m_pid_hrelay;
    int m_pid_evs;
    int m_nnodes;
    char m_nodename[256];

  };

}
#endif




