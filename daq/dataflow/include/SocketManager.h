#ifndef SOCKETMAN_H
#define SOCKETMAN_H
//+
// File : SocketManager.h
// Description : Class to manage multiple socket connection using select()
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 19 - Feb - 2013
//-

#include <vector>
#include <sys/select.h>

namespace Belle2 {

  class SocketManager {
  public:
    SocketManager(int sock);
    ~SocketManager();

    int examine();

    std::vector<int>& connected_socket_list();

    bool connected(int fd, bool wreq = false);

    bool remove(int fd);

  private:
    int m_maxfd;
    std::vector<int> m_fd;

    int m_sock;
    fd_set m_rset;
    fd_set m_wset;
    fd_set m_allset;
  };
};
#endif






