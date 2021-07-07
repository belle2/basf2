/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef SOCKETLIB_H
#define SOCKETLIB_H

#include <netdb.h>
#include <netinet/in.h>

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






