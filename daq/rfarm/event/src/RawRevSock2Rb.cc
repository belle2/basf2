/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/rfarm/event/RawRevSock2Rb.h"
#include "daq/dataflow/REvtSocket.h"

#include <unistd.h>

using namespace std;
using namespace Belle2;

RawRevSock2Rb::RawRevSock2Rb(string rbuf, string src, int port, string shmname, int id)
{
  //  m_rbuf = new RingBuffer(rbuf.c_str(), RBUFSIZE);
  m_rbuf = new RingBuffer(rbuf.c_str());
  m_sock = new RSocketRecv(src.c_str(), (u_int)port);
  m_evtbuf = new char[MAXEVTSIZE];
  m_flow = new RFFlowStat((char*)shmname.c_str(), id, m_rbuf);
  m_buf = new int[MAXBUFSIZE];

}

RawRevSock2Rb::~RawRevSock2Rb(void)
{
  delete m_sock;
  delete m_rbuf;
}

int RawRevSock2Rb::ReceiveEvent(void)
{
  // Get a record from socket
  int bufsize = m_sock->get_wordbuf(m_buf, MAXBUFSIZE);

  if (bufsize <= 0) {
    return 0;
  }
  m_flow->log(bufsize * 4);

  // Put the message in ring buffer
  int stat = 0;
  for (;;) {
    stat = m_rbuf->insq(m_buf, bufsize);
    if (stat >= 0) break;
    //    usleep(100);
    usleep(20);
  }
  return stat;
}

int RawRevSock2Rb::Reconnect(int ntimes)
{
  return m_sock->reconnect(ntimes);
}


