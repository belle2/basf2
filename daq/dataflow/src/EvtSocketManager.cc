/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/dataflow/EvtSocketManager.h>

#include <algorithm>
#include <cstring>
#include <unistd.h>

using namespace Belle2;
using namespace std;

EvtSocketManager::EvtSocketManager(EvtSocketRecv* evtsock)
{
  SocketRecv* sock = evtsock->sock();

  FD_ZERO(&m_allset);
  FD_SET(sock->sock(), &m_allset);
  m_maxfd = sock->sock();
  m_recv = evtsock;
  m_sock = sock->sock();
}

EvtSocketManager::~EvtSocketManager()
{
}

int EvtSocketManager::examine()
{
  memcpy(&m_rset, &m_allset, sizeof(m_rset));
  memcpy(&m_wset, &m_allset, sizeof(m_wset));
  struct timeval timeout;
  timeout.tv_sec = 0; // 1sec
  timeout.tv_usec = 1000; // 1msec (in microsec)
  //  printf ( "EvtSocketManager: maxfd = %d, start select...\n", m_maxfd);
  //  int rc = select(m_maxfd+1, &m_rset, &m_wset, NULL, NULL );
  int rc = select(m_maxfd + 1, &m_rset, &m_wset, NULL, &timeout);
  //  printf ( "EvtSocketManager: select returned with %d\n", rc );
  if (rc < 0)
    perror("select");
  else if (rc == 0)   // timeout
    return -1;

  if (FD_ISSET(m_sock, &m_rset)) { // New connection request
    int fd  = (m_recv->sock())->accept();
    m_fd.push_back(fd);
    FD_SET(fd, &m_allset);
    if (fd > m_maxfd) m_maxfd = fd;
    /*
    struct sockaddr_in peername;
    socklen_t lname = sizeof (peername);
    int is = getpeername ( fd[0], (sockaddr*)&peername, &lname );
    struct hostent* dest = gethostbyaddr (
    (char*)&peername.sin_addr.s_addr, sizeof(unsigned int),AF_INET );
    tprintf ( "DQM_server : connected to host = %s (port %d); fd=%d\n",
        dest->h_name, ntohs(peername.sin_port), fd[0] );
    */
    return 0;
  } else { //
    return 1;
  }
}

vector<int>& EvtSocketManager::connected_socket_list()
{
  return m_fd;
}

bool EvtSocketManager::connected(int fd, bool wreq)
{
  if (!wreq) {
    if (FD_ISSET(fd, &m_rset))
      return true;
    else
      return false;
  } else {
    if (FD_ISSET(fd, &m_wset))
      return true;
    else
      return false;
  }
}

bool EvtSocketManager::remove(int fd)
{
  FD_CLR(fd, &m_allset);
  shutdown(fd, SHUT_RDWR);
  close(fd);
  vector<int>::iterator pos = find(m_fd.begin(), m_fd.end(), fd);
  if (pos != m_fd.end()) {
    m_fd.erase(pos);
    return true;
  } else
    return false;
}





