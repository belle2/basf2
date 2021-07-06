/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef EVTSOCKET_H
#define EVTSOCKET_H

#include <string>

#include <daq/dataflow/SocketLib.h>
#include <framework/pcore/EvtMessage.h>

#define MAXEVTSIZE 80000000

namespace Belle2 {

  typedef unsigned short u_short;

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

}

#endif


