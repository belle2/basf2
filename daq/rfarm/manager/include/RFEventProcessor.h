#ifndef RFEVENTPROCESSOR_H
#define RFEVENTPROCESSOR_H
//+
// File : RFEventProcessor.h
// Description : receive event, process it by basf2, and send output
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

  class RFEventProcessor : public RFServerBase {
  public:
    RFEventProcessor(std::string conffile);
    ~RFEventProcessor();

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

    int m_pid_receiver;
    int m_pid_basf2
    int m_pid_sender;
    int m_nnodes;

  };

}
#endif




