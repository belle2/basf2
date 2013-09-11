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
    void Configure(NSMmsg*, NSMcontext*);
    void Start(NSMmsg*, NSMcontext*);
    void Stop(NSMmsg*, NSMcontext*);
    void Restart(NSMmsg*, NSMcontext*);

    // Server function
    void server();

  private:
    RFConf*            m_conf;
    RFProcessManager*  m_proc;
    RFLogManager*      m_log;

    int m_pid_dqm;

  };
}
#endif

