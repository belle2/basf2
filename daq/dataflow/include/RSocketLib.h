#ifndef RSOCKETLIB_H
#define RSOCKETLIB_H
//+
// File : socketlib.h
// Description : Class to handle a socket
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 18 - Aug - 2000
//-

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ipc.h>

#include <string>

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






