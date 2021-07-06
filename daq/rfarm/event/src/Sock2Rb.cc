/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/rfarm/event/Sock2Rb.h"

#include <unistd.h>

using namespace std;
using namespace Belle2;

Sock2Rb::Sock2Rb(string rbuf, int port)
{
  //  m_rbuf = new RingBuffer(rbuf.c_str(), RBUFSIZE);
  m_rbuf = new RingBuffer(rbuf.c_str());
  m_sock = new EvtSocketRecv(port);
  m_evtbuf = new char[MAXEVTSIZE];

}

Sock2Rb::~Sock2Rb(void)
{
  delete m_sock;
  delete m_rbuf;
}

int Sock2Rb::ReceiveEvent(void)
{
  // Get a record from socket
  EvtMessage* msg = m_sock->recv();
  if (msg == NULL) {
    return 0;
  }
  //  B2INFO ( "Rx: got an event from Socket, size=" << msg->size()  );
  if (msg->type() == MSG_TERMINATE) {
    return 0;
    // Flag End Of File !!!!!
    //    return msg->type(); // EOF
  }

  // Put the message in ring buffer
  int stat = 0;
  for (;;) {
    stat = m_rbuf->insq((int*)msg->buffer(), msg->paddedSize());
    if (stat >= 0) break;
    //    usleep(100);
    usleep(20);
  }
  delete msg;
  return stat;
}

