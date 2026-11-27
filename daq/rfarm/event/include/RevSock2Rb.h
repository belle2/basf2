/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

#include "framework/pcore/RingBuffer.h"
#include "daq/dataflow/REvtSocket.h"
#include "daq/rfarm/manager/RFFlowStat.h"

#define RBUFSIZE 100000000

namespace Belle2 {

  class RevSock2Rb {
  public:
    /*! Constuctor and Destructor */
    RevSock2Rb(const std::string& rbuf, std::string src, int port, std::string shmname, int id);
    ~RevSock2Rb(void);
    /** Copy constructor (disabled). */
    RevSock2Rb(const RevSock2Rb&) = delete;

    /** Operator = (disabled). */
    RevSock2Rb& operator=(const RevSock2Rb&) = delete;

    /*! Event function */
    int ReceiveEvent(void);

    /*! Reconnect */
    int Reconnect(int ntry);

  private:
    RingBuffer* m_rbuf;
    REvtSocketRecv* m_sock;
    RFFlowStat* m_flow;
    char* m_evtbuf;

  };

}
