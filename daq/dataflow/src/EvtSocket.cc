//+
// File : EvtSocket.cc
// Description : Socket interface to transver EvtMessage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 5 - Aug - 2011
//-

#include "daq/dataflow/EvtSocket.h"

using namespace Belle2;

// EvtSocketSend

EvtSocketSend::EvtSocketSend(std::string host, int port)
{
  m_sock = new SocketSend(host.c_str(), (u_short)port);
  m_recbuf = new char[MAXEVTSIZE];
}

EvtSocketSend::~EvtSocketSend()
{
  delete m_sock;
  delete[] m_recbuf;
}

int EvtSocketSend::send(EvtMessage* msg)
{
  return m_sock->put((char*)msg->buffer(), msg->size());
}

EvtMessage* EvtSocketSend::recv()
{
  int stat = m_sock->get(m_recbuf, MAXEVTSIZE);
  if (stat <= 0) return NULL;
  EvtMessage* evt = new EvtMessage(m_recbuf);
  //  delete [] evtbuf;
  return evt;
}

int EvtSocketSend::send_buffer(int nbytes, char* buf)
{
  return m_sock->put(buf, nbytes);
}

int EvtSocketSend::recv_buffer(char* buf)
{
  int stat = m_sock->get(buf, MAXEVTSIZE);
  return stat;
}

// EvtSocketRecv

EvtSocketRecv::EvtSocketRecv(int port)
{
  m_sock = new SocketRecv((u_short)port);
  m_recbuf = new char[MAXEVTSIZE];
  m_sock->accept();
}

EvtSocketRecv::~EvtSocketRecv()
{
  delete m_sock;
  delete[] m_recbuf;
}

EvtMessage* EvtSocketRecv::EvtSocketRecv::recv()
{
  int stat = m_sock->get(m_recbuf, MAXEVTSIZE);
  if (stat <= 0) return NULL;
  EvtMessage* evt = new EvtMessage(m_recbuf);
  //  delete [] evtbuf;
  return evt;
}

int EvtSocketRecv::send(EvtMessage* msg)
{
  return m_sock->put((char*)msg->buffer(), msg->size());
}

int EvtSocketRecv::send_buffer(int nbytes, char* buf)
{
  return m_sock->put(buf, nbytes);
}

int EvtSocketRecv::recv_buffer(char* buf)
{
  int stat = m_sock->get(buf, MAXEVTSIZE);
  return stat;
}
