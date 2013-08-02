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

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFSharedMem.h"
#include "daq/rfarm/manager/RFProcessManager.h"
#include "daq/rfarm/manager/RFLogManager.h"
#include "daq/rfarm/manager/RFNSM.h"

#include "daq/rfarm/manager/RFServerBase.h"

#define MAXNODES 256

namespace Belle2 {

  class RFOutputServer : public RFServerBase {
  public:
    RFOutputServer(std::string conffile);
    ~RFOutputServer();

    // Functions to be hooked to NSM
    void Configure(NSMmsg*, NSMcontext*);
    void Start(NSMmsg*, NSMcontext*);
    void Stop(NSMmsg*, NSMcontext*);
    void Restart(NSMmsg*, NSMcontext*);

    // Server function
    void server();

  private:
    RFConf*            m_conf;
    RFSharedMem*       m_shm;
    RFProcessManager*  m_proc;
    RFLogManager*      m_log;

    int m_pid_receiver[MAXNODES];
    int m_pid_basf2;
    int m_pid_sender;
    int m_nnodes;

  };

}
#endif




