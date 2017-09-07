#ifndef ERECO_EVENT_SAMPLER_H
#define ERECO_EVENT_SAMPLER_H
//+
// File : ERecoEventSampler.h
// Description : Fetch events from event processes and place them in a RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 17 - Dec - 2013
//-

#include <string>
#include <vector>

#include <daq/rfarm/manager/RFConf.h>
#include <daq/rfarm/manager/RFProcessManager.h>
#include <daq/rfarm/manager/RFLogManager.h>
#include <daq/rfarm/manager/RFNSM.h>

#include <daq/dataflow/SocketLib.h>
#include <daq/dataflow/EvtSocket.h>
#include <daq/dataflow/EvtSocketManager.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/MsgHandler.h>

#define MAXEVTSIZE 80000000

namespace Belle2 {
  class ERecoEventSampler : public RFServerBase {
  public:
    ERecoEventSampler(std::string conffile);
    ~ERecoEventSampler();

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
    std::string        m_conffile;
    RFProcessManager*  m_proc;
    RFLogManager*      m_log;
    RingBuffer*        m_rbufout;

    int m_pid_sampler;
    int m_pid_server;

  };

}
#endif
