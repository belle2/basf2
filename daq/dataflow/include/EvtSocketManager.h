#ifndef EVTSOCKETMAN_H
#define EVTSOCKETMAN_H
//+
// File : EvtSocketManager.h
// Description : Class to manage multiple EvtSocket connection using select()
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 19 - Feb - 2013
//-

#include <string>
#include <vector>
#include <algorithm>

#include <daq/dataflow/EvtSocket.h>

namespace Belle2 {

  class EvtSocketManager {
  public:
    EvtSocketManager(EvtSocketRecv* sock);
    ~EvtSocketManager();

    int examine();

    std::vector<int>& connected_socket_list();

    bool connected(int fd, bool wreq = false);

    bool remove(int fd);

  private:
    int m_maxfd;
    std::vector<int> m_fd;

    EvtSocketRecv* m_recv;
    int m_sock;
    fd_set m_rset;
    fd_set m_wset;
    fd_set m_allset;
  };
};
#endif






