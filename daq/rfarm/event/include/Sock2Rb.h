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

  class Sock2Rb {
  public:
    /*! Constuctor and Destructor */
    Sock2Rb(std::string rbuf, int port);
    ~Sock2Rb(void);

    /*! Event function */
    int ReceiveEvent(void);

  private:
    RingBuffer* m_rbuf;
    EvtSocketRecv* m_sock;
    char* m_evtbuf;

  };

}
