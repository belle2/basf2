//+
// File : SocketManager.cc
// Description : Manage incoming socket connections using select()
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 19 - Feb - 2013
//-


#include <daq/dataflow/SocketManager.h>

using namespace Belle2;
using namespace std;

SocketManager::SocketManager(int sock)
{

  FD_ZERO(&m_allset);
  FD_SET(sock, &m_allset);
  m_maxfd = sock;
  m_sock = sock;
}

SocketManager::~SocketManager()
{
}

int SocketManager::examine()
{
  //  sleep ( 1 );
  memcpy(&m_rset, &m_allset, sizeof(m_rset));
  memcpy(&m_wset, &m_allset, sizeof(m_wset));

  struct timeval timeout;
  timeout.tv_sec = 0; // 1sec
  timeout.tv_usec = 1000; // 1msec (in microsec)
  //  printf ( "SocketManager: maxfd = %d, start select...; rset=%x, wset=%x\n", m_maxfd, m_rset, m_wset);

  int rc = select(m_maxfd + 1, &m_rset, NULL, NULL, NULL);
  //  int rc = select(m_maxfd + 1, &m_rset, &m_wset, NULL, NULL);
  //  int rc = select(m_maxfd + 1, &m_rset, &m_wset, NULL, &timeout);
  //  printf ( "SocketManager: select returned with %d; rset=%x, wset=%x\n", rc, m_rset, m_wset );
  /*
  for ( int i=0;i<m_maxfd+1; i++ ) {
    printf ( "Checking socket %d ", i );
    if ( FD_ISSET( i, &m_rset) ) printf ( "Rset is set " );
    if ( FD_ISSET( i, &m_wset) ) printf ( "Wset is set " );
    printf ( "\n" );
  }
  */

  if (rc < 0)
    perror("select");
  else if (rc == 0)   // timeout
    return -1;

  // Accept if new connection request
  int t;
  if (FD_ISSET(m_sock, &m_rset)) { // New connection request
    //    int fd  = (m_recv->sock())->accept();

    struct sockaddr_in isa;
    socklen_t i = sizeof(isa);
    getsockname(m_sock, (struct sockaddr*)&isa, &i);
    if ((t =::accept(m_sock, (struct sockaddr*)&isa, &i)) < 0) {
      //      m_errno = errno;
      return (-1);
    }
    m_fd.push_back(t);
    printf("New socket registered t=%d, list size = %d\n", t, m_fd.size());
    FD_SET(t, &m_allset);
    if (t > m_maxfd) m_maxfd = t;

    return 0;
  } else { //
    return 1;
  }
}

vector<int>& SocketManager::connected_socket_list()
{
  return m_fd;
}

bool SocketManager::connected(int fd, bool wreq)
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

bool SocketManager::remove(int fd)
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





