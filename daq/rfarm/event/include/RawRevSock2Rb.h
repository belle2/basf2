/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <string>

#include "framework/pcore/RingBuffer.h"
#include "daq/dataflow/RSocketLib.h"
#include "daq/rfarm/manager/RFFlowStat.h"

#define MAXBUFSIZE 20000000
#define RBUFSIZE 100000000

class RawRevSock2Rb {
public:
  /*! Constuctor and Destructor */
  RawRevSock2Rb(std::string rbuf, std::string src, int port, std::string shmname, int id);
  ~RawRevSock2Rb(void);

  /*! Event function */
  int ReceiveEvent(void);

  /*! Reconnect */
  int Reconnect(int ntry);

private:
  Belle2::RingBuffer* m_rbuf;
  Belle2::RSocketRecv* m_sock;
  Belle2::RFFlowStat* m_flow;
  char* m_evtbuf;
  int* m_buf;

};

