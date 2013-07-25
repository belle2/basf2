#ifndef REVTSOCKET_H
#define REVTSOCKET_H
//+
// File : REvtSocket.h
// Description : Socket I/O interface for EvtMessage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 4 - Aug - 2011
//-

#include <string>

#include <daq/dataflow/RSocketLib.h>
#include <framework/pcore/EvtMessage.h>

#define MAXEVTSIZE 80000000

typedef unsigned short u_short;

using namespace Belle2;

class REvtSocketRecv {
public:
  REvtSocketRecv(std::string hostname, int port);
  ~REvtSocketRecv();

  int status();

  int send(EvtMessage* msg);
  EvtMessage* recv(void);

  int send_buffer(int size, char* buf);
  int recv_buffer(char* buf);

  RSocketRecv* sock(void);

private:
  RSocketRecv* m_sock;
  char* m_recbuf;
};

class REvtSocketSend {
public:
  REvtSocketSend(int port, bool accept_at_init = true);
  ~REvtSocketSend();

  int status();

  int send(EvtMessage* msg);
  EvtMessage* recv(void);

  int send_buffer(int size, char* buf);
  int recv_buffer(char* buf);

  RSocketSend* sock(void);

private:
  RSocketSend* m_sock;
  char* m_recbuf;
};
#endif


