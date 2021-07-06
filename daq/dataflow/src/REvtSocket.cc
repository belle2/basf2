/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/dataflow/REvtSocket.h"

using namespace Belle2;

// REvtSocketRecv

REvtSocketRecv::REvtSocketRecv(std::string host, int port)
{
  m_sock = new RSocketRecv(host.c_str(), (u_short)port);
  m_recbuf = new char[MAXEVTSIZE];
}

REvtSocketRecv::~REvtSocketRecv()
{
  delete m_sock;
  delete[] m_recbuf;
}

int REvtSocketRecv::send(EvtMessage* msg)
{
  //  printf ( "REvtSocketRecv : sending = %d\n", msg->size() );
  return m_sock->put((char*)msg->buffer(), msg->size());
}

EvtMessage* REvtSocketRecv::recv()
{
  int stat = m_sock->get(m_recbuf, MAXEVTSIZE);
  if (stat <= 0) return NULL;
  EvtMessage* evt = new EvtMessage(m_recbuf);
  //  delete [] evtbuf;
  return evt;
}

int REvtSocketRecv::send_buffer(int nbytes, char* buf)
{
  return m_sock->put(buf, nbytes);
}

int REvtSocketRecv::recv_buffer(char* buf)
{
  int stat = m_sock->get(buf, MAXEVTSIZE);
  return stat;
}

RSocketRecv* REvtSocketRecv::sock(void)
{
  return m_sock;
}

// REvtSocketSend

REvtSocketSend::REvtSocketSend(int port, bool accept_at_init)
{
  m_sock = new RSocketSend((u_short)port);
  m_recbuf = new char[MAXEVTSIZE];
  if (accept_at_init)
    m_sock->accept();
}

REvtSocketSend::~REvtSocketSend()
{
  delete m_sock;
  delete[] m_recbuf;
}

EvtMessage* REvtSocketSend::REvtSocketSend::recv()
{
  int stat = m_sock->get(m_recbuf, MAXEVTSIZE);
  if (stat <= 0) return NULL;
  EvtMessage* evt = new EvtMessage(m_recbuf);
  //  delete [] evtbuf;
  return evt;
}

int REvtSocketSend::send(EvtMessage* msg)
{
  return m_sock->put((char*)msg->buffer(), msg->size());
}

int REvtSocketSend::send_buffer(int nbytes, char* buf)
{
  return m_sock->put(buf, nbytes);
}

int REvtSocketSend::recv_buffer(char* buf)
{
  int stat = m_sock->get(buf, MAXEVTSIZE);
  return stat;
}

RSocketSend* REvtSocketSend::sock(void)
{
  return m_sock;
}
