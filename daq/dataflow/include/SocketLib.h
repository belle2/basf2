#ifndef SOCKETLIB_H
#define SOCKETLIB_H
//+
// File : socketlib.h
// Description : Class to handle a socket
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 18 - Aug - 2000
//-

#include <string>

#define D2_SOCKBUF_SIZE  8000000

typedef unsigned short u_short;

namespace Belle2 {


  class SocketIO {
  public:
    SocketIO();
    ~SocketIO();

    int put(int sock, char* data, int len);
    int get(int sock, char* data, int len);

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

    int examine();
    int accept();
    int close();

    int sock() const;
    int sender() const;
    void sock(int sockid);

    int get(char* data, int len);
    int read(char* data, int len);
    int put(char* data, int len);
    int write(char* data, int len);

    int err() const;
    void interrupt();

  private:
    SocketIO m_io;
    int m_sock;
    int m_sender;
    int m_errno;
    int m_int;

  };

  class SocketSend : public SocketIO {
  public:
    SocketSend(const char* node, u_short prt);
    ~SocketSend();

    int put(char* data, int len);
    int write(char* data, int len);
    int get(char* data, int len);
    int read(char* data, int len);

    char* node(void);
    int port(void);
    int sock(void);
    void sock(int sockid);

    int err(void);

  private:
    SocketIO m_io;
    int m_sock;
    int m_port;
    char m_node[128];
    int m_errno;

  };

}
#endif






