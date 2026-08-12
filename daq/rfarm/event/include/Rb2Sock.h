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
#include "daq/dataflow/EvtSocket.h"

#define RBUFSIZE 100000000

namespace Belle2 {

  class Rb2Sock {
  public:
    /*! Constuctor and Destructor */
    Rb2Sock(const std::string& rbuf, std::string dest, int port);
    ~Rb2Sock(void);
    /** Copy constructor (disabled). */
    Rb2Sock(const Rb2Sock&) = delete;

    /** Operator = (disabled). */
    Rb2Sock& operator=(const Rb2Sock&) = delete;

    /*! Event function */
    int SendEvent(void);

  private:
    RingBuffer* m_rbuf;
    EvtSocketSend* m_sock;
    char* m_evtbuf;

  };

}
