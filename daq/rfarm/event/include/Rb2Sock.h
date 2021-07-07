/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <string>

#include "framework/pcore/RingBuffer.h"
#include "daq/dataflow/EvtSocket.h"

#define RBUFSIZE 100000000

namespace Belle2 {

  class Rb2Sock {
  public:
    /*! Constuctor and Destructor */
    Rb2Sock(std::string rbuf, std::string dest, int port);
    ~Rb2Sock(void);

    /*! Event function */
    int SendEvent(void);

  private:
    RingBuffer* m_rbuf;
    EvtSocketSend* m_sock;
    char* m_evtbuf;

  };

}
