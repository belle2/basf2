/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef EVENT_SAMPLER_H
#define EVENT_SAMPLER_H

#include <string>
#include <vector>

#include <daq/dataflow/EvtSocket.h>
#include <framework/pcore/RingBuffer.h>

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
