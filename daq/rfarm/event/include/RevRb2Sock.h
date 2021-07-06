/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <string>

#include "framework/pcore/RingBuffer.h"
#include "daq/dataflow/REvtSocket.h"
#include "daq/rfarm/manager/RFFlowStat.h"

#define RBUFSIZE 100000000

namespace Belle2 {

  class RevRb2Sock {
  public:
    /*! Constuctor and Destructor */
    RevRb2Sock(std::string rbuf, int port, std::string shmname, int id);
    ~RevRb2Sock(void);

    /*! Event function */
    int SendEvent(void);

  private:
    RingBuffer* m_rbuf;
    REvtSocketSend* m_sock;
    RFFlowStat* m_flow;
    char* m_evtbuf;

  };

}
