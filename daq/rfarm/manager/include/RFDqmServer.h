#ifndef RF_DQMSERVER_H
#define RF_DQMSERVER_H
//+
// File : RFDqmServer.h
// Description : DQM server for RFARM
//
// Author : Ryosuke Itoh, KEK
// Date : 14 - Jun - 2013
//-

#include <string>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFProcessManager.h"
#include "daq/rfarm/manager/RFLogManager.h"
#include "daq/rfarm/manager/RFNSM.h"

#include "daq/rfarm/manager/RFServerBase.h"

#define MAXNODES 256

namespace Belle2 {

  class RFDqmServer : public RFServerBase {
  public:
    RFDqmServer(std::string conffile);
    ~RFDqmServer();

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
    RFProcessManager*  m_proc;
    RFLogManager*      m_log;

    int m_pid_dqm;
    int m_pid_relay;

    int m_expno;
    int m_runno;

  };
}
#endif

