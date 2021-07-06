/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef RSOCKETLIB_H
#define RSOCKETLIB_H

#include <netinet/in.h>

#define D2_SOCKBUF_SIZE  8000000

#include "daq/dataflow/SocketLib.h"

typedef unsigned short u_short;

namespace Belle2 {

  class RSocketSend : public SocketIO {
  public:
    RSocketSend(u_short port);
    ~RSocketSend();

    int examine();
    int accept();
    int close();

    int sock() const;
    int sender() const;
    void sock(int sockid);
    int port() const;

    int get(char* data, int len);
    int read(char* data, int len);
    int put(char* data, int len);
    int put_wordbuf(int* data, int len);
    int write(char* data, int len);

    int err() const;
    void interrupt();

  private:
    SocketIO m_io;
    int m_sock;
    int m_port;
    int m_sender;
    int m_errno;
    int m_int;

  };

  class RSocketRecv : public SocketIO {
  public:
    RSocketRecv(const char* node, u_short prt);
    ~RSocketRecv();

    int reconnect(int ntry);

    int put(char* data, int len);
    int write(char* data, int len);
    int get(char* data, int len);
    int get_wordbuf(int* data, int len);
    int read(char* data, int len);

    char* node(void);
    int port(void);
    int sock(void);
    void sock(int sockid);

    int err(void);

  private:
    SocketIO m_io;
    struct hostent* m_hp;
    struct sockaddr_in m_sa;
    int m_sock;
    int m_port;
    char m_node[128];
    int m_errno;

  };

}
#endif






