/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef REVTSOCKET_H
#define REVTSOCKET_H

#include <string>

#include <daq/dataflow/RSocketLib.h>
#include <framework/pcore/EvtMessage.h>

#define MAXEVTSIZE 80000000

namespace Belle2 {

  typedef unsigned short u_short;

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

}

#endif


