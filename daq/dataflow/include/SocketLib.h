#ifndef SOCKETLIB_H
#define SOCKETLIB_H
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

typedef unsigned short u_short;

namespace Belle2 {


  class SocketIO {
  public:
    SocketIO();
    ~SocketIO();

    int put(int sock, char* data, int len);
    int put_wordbuf(int sock, int* data, int len);
    int get(int sock, char* data, int len);
    int get_wordbuf(int sock, int* data, int len);

    int get_pxd(int sock, char* data, int len);

    // protected:
    int read_data(int sock, char* data, int len);
    int write_data(int sock, char* data, int len);

    void interrupt();

  private:
    int m_int;

  };

  class SocketRecv : public SocketIO {
  public:
    SocketRecv(u_short port);
    ~SocketRecv();

    int reconnect(int ntry);

    int examine();
    int accept();
    int close();

    int sock() const;
    int sender() const;
    void sock(int sockid);

    int get(char* data, int len);
    int get_wordbuf(int* data, int len);
    int read(char* data, int len);
    int put(char* data, int len);
    int write(char* data, int len);

    int err() const;
    void interrupt();

  private:
    SocketIO m_io;
    struct hostent* m_hp;
    struct sockaddr_in m_sa;
    int m_port;
    int m_sock;
    int m_sender;
    int m_errno;
    int m_int;

  };

  class SocketSend : public SocketIO {
  public:
    SocketSend(const char* node, u_short prt);
    ~SocketSend();

    int reconnect(int ntry);

    int put(char* data, int len);
    int put_wordbuf(int* data, int len);
    int write(char* data, int len);
    int get(char* data, int len);
    int get_pxd(char* data, int len);
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






