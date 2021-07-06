/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef EVTSOCKETMAN_H
#define EVTSOCKETMAN_H

#include <vector>

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






