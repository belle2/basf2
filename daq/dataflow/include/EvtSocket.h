#ifndef EVTSOCKET_H
#define EVTSOCKET_H
//+
// File : EvtSocket.h
// Description : Socket I/O interface for EvtMessage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 4 - Aug - 2011
//-

#include <string>

#include <daq/dataflow/SocketLib.h>
#include <framework/pcore/EvtMessage.h>

#define MAXEVTSIZE 80000000

typedef unsigned short u_short;

using namespace Belle2;

class EvtSocketSend {
public:
  EvtSocketSend(std::string hostname, int port);
  ~EvtSocketSend();

  int status();

  int send(EvtMessage* msg);
  EvtMessage* recv(void);

  int send_buffer(int size, char* buf);
  int recv_buffer(char* buf);
  int recv_pxd_buffer(char* buf);

  SocketSend* sock(void);

private:
  SocketSend* m_sock;
  char* m_recbuf;
};

class EvtSocketRecv {
public:
  EvtSocketRecv(int port, bool accept_at_init = true);
  ~EvtSocketRecv();

  int status();

  int send(EvtMessage* msg);
  EvtMessage* recv(void);

  int send_buffer(int size, char* buf);
  int recv_buffer(char* buf);

  SocketRecv* sock(void);

private:
  SocketRecv* m_sock;
  char* m_recbuf;
};
#endif


