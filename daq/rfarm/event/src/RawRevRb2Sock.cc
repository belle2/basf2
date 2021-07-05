/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/rfarm/event/RawRevRb2Sock.h"

using namespace std;
using namespace Belle2;

RawRevRb2Sock::RawRevRb2Sock(string rbuf, int port, string shmname, int id)
{
  //  m_rbuf = new RingBuffer((char*)rbuf.c_str(), RBUFSIZE);
  m_rbuf = new RingBuffer((char*)rbuf.c_str());
  m_sock = new RSocketSend(port);
  m_evtbuf = new int[MAXEVTSIZE];
  m_flow = new RFFlowStat((char*)shmname.c_str(), id, m_rbuf);

  m_sock->accept();
}

RawRevRb2Sock::~RawRevRb2Sock(void)
{
  delete m_sock;
  delete m_rbuf;
}

int RawRevRb2Sock::SendEvent(void)
{
  // Get a record from ringbuf
  int size;
  while ((size = m_rbuf->remq(m_evtbuf)) == 0) {
    //    printf ( "Rx : evtbuf is not available yet....\n" );
    //    usleep(100);
    usleep(20);
  }
  m_flow->log(size * 4);

  int is = m_sock->put_wordbuf(m_evtbuf, size);
  //  if (is < 0) perror("put_wordbuf");
  return is;
}

int RawRevRb2Sock::Reconnect(void)
{
  int port = m_sock->port();
  delete m_sock;
  m_sock = new RSocketSend((u_short)port);
  m_sock->accept();
}


