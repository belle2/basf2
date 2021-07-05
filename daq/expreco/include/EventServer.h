/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef EVENT_SERVER_H
#define EVENT_SERVER_H

#include <string>

#include <daq/dataflow/EvtSocket.h>
#include <daq/dataflow/EvtSocketManager.h>
#include <framework/pcore/RingBuffer.h>

#define MAXBUFSIZE 80000000

namespace Belle2 {

  class EventServer {
  public:
    EventServer(std::string rbufname, int port);
    ~EventServer();

    int server();  // Loop

  private:
    EvtSocketRecv* m_sock;
    EvtSocketManager* m_man;
    RingBuffer* m_rbuf;
    int m_port;
    int m_force_exit;
  };
}
#endif
