/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

#include <vector>
#include <string>

#include "framework/pcore/RingBuffer.h"
#include "daq/dataflow/REvtSocket.h"
#include "daq/rfarm/manager/RFFlowStat.h"

#define RBUFSIZE 100000000

namespace Belle2 {

  class RawRevRb2Sock {
  public:
    /*! Constuctor and Destructor */
    RawRevRb2Sock(std::string rbuf, int port, std::string shmname, int id);
    ~RawRevRb2Sock(void);

    /*! Event function */
    int SendEvent(void);

    /*! Reconnect */
    int Reconnect(void);

  private:
    RingBuffer* m_rbuf;
    RSocketSend* m_sock;
    RFFlowStat* m_flow;
    int* m_evtbuf;

  };

}
