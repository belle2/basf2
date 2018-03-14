#ifndef EVENT_SAMPLER_H
#define EVENT_SAMPLER_H
//+
// File : EventSampler.h
// Description : Fetch events from event processes and place them in a RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 31 - Jul - 2017
//-

#include <string>
#include <vector>

#include <daq/dataflow/SocketLib.h>
#include <daq/dataflow/EvtSocket.h>
#include <daq/dataflow/EvtSocketManager.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/MsgHandler.h>
#include <daq/rfarm/manager/RFConf.h>

#define MAXEVTSIZE 80000000

namespace Belle2 {

  class EventSampler {
  public:
    EventSampler(std::vector<std::string> nodes, int port, std::string rbufname, int interval = 5000);
    ~EventSampler();
    int server();  // Loop

  private:
    std::vector<EvtSocketSend*> m_socklist;
    RingBuffer* m_rbuf;
    int m_port;
    int m_force_exit;
    int m_interval;
  };
}
#endif
