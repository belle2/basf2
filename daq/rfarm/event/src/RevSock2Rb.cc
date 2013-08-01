//+
// File : RevSock2Rb.cc
// Description : Receive events from Socket and place them in Rbuf
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 26 - Apr - 2012
//-

#include "daq/rfarm/event/RevSock2Rb.h"

using namespace std;
using namespace Belle2;

RevSock2Rb::RevSock2Rb(string rbuf, string src, int port)
{
  m_rbuf = new RingBuffer(rbuf.c_str(), RBUFSIZE);
  m_sock = new REvtSocketRecv(src, port);
  m_evtbuf = new char[MAXEVTSIZE];

}

RevSock2Rb::~RevSock2Rb(void)
{
  delete m_sock;
  delete m_rbuf;
}

int RevSock2Rb::ReceiveEvent(void)
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
    stat = m_rbuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
    if (stat >= 0) break;
    usleep(200);
  }
  return stat;
}

