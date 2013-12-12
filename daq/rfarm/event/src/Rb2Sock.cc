//+
// File : Rb2Sock.cc
// Description : Send events in RingBuffer to Socket
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 26 - Apr - 2012
//-

#include "daq/rfarm/event/Rb2Sock.h"

using namespace std;
using namespace Belle2;

Rb2Sock::Rb2Sock(string rbuf, string dest, int port)
{
  //  m_rbuf = new RingBuffer(rbuf.c_str(), RBUFSIZE);
  m_rbuf = new RingBuffer(rbuf.c_str());
  m_sock = new EvtSocketSend(dest, port);
  m_evtbuf = new char[MAXEVTSIZE];

}

Rb2Sock::~Rb2Sock(void)
{
  delete m_sock;
  delete m_rbuf;
}

int Rb2Sock::SendEvent(void)
{
  // Get a record from ringbuf
  int size;
  while ((size = m_rbuf->remq((int*)m_evtbuf)) == 0) {
    //    printf ( "Rx : evtbuf is not available yet....\n" );
    //    usleep(100);
    usleep(20);
  }

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

