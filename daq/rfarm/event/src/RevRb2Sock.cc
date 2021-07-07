/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/rfarm/event/RevRb2Sock.h"

#include <unistd.h>

using namespace std;
using namespace Belle2;

RevRb2Sock::RevRb2Sock(string rbuf, int port, string shmname, int id)
{
  //  m_rbuf = new RingBuffer((char*)rbuf.c_str(), RBUFSIZE);
  m_rbuf = new RingBuffer((char*)rbuf.c_str());
  m_sock = new REvtSocketSend(port);
  m_evtbuf = new char[MAXEVTSIZE];
  m_flow = new RFFlowStat((char*)shmname.c_str(), id, m_rbuf);

}

RevRb2Sock::~RevRb2Sock(void)
{
  delete m_sock;
  delete m_rbuf;
}

int RevRb2Sock::SendEvent(void)
{
  // Get a record from ringbuf
  int size;
  while ((size = m_rbuf->remq((int*)m_evtbuf)) == 0) {
    //    printf ( "Rx : evtbuf is not available yet....\n" );
    //    usleep(100);
    usleep(20);
  }
  m_flow->log(size * 4);

  EvtMessage* msg = new EvtMessage(m_evtbuf);    // Ptr copy, no overhead

  if (msg->type() == MSG_TERMINATE) {
    printf("EoF found. Exitting.....\n");
    m_sock->send(msg);
    delete msg;
    return -1;
  } else {
    int is = m_sock->send(msg);
    delete msg;
    return is;
    //    return msg->size();
  }
}

