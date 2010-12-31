/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include "string.h"

#include <daq/hlt/B2Socket.h>

using namespace Belle2;

/* @brief B2Socket constructor
 *
 * Initializing related variables and allocating memory space
*/
B2Socket::B2Socket() : m_sock(-1), m_maxhosts(-1), m_maxcons(-1), m_maxrecv(MAXPACKETSIZE)
{
  memset(&m_addr, 0, sizeof(m_addr));
}

/// @brief B2Socket constructor. Initializing related variables and allocating memory space
/// @param maxhosts Number of hosts allowed in maximum
/// @param maxcons Number of connections in maximum
/// @param maxrecv Maximum receiving buffer size in bytes
B2Socket::B2Socket(int maxhosts, int maxcons, int maxrecv)
    : m_sock(-1), m_maxhosts(maxhosts), m_maxcons(maxcons), m_maxrecv(maxrecv)
{
  memset(&m_addr, 0, sizeof(m_addr));
}

/// @brief B2Socket destructor. Closing the socket
B2Socket::~B2Socket()
{
  if (is_valid())
    ::close(m_sock);
}

/// @brief Socket creator. Open socket as AF_INET
/// @return c_Success Socket creation success
/// @return c_InitFailed Socket creation failed
EStatus B2Socket::create()
{
  m_sock = socket(AF_INET, SOCK_STREAM, 0);

  if (!is_valid()) {
    B2ERROR("Socket creation failed");
    return c_InitFailed;
  }

  int on = 1;
  if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) == -1)
    return c_InitFailed;

  B2DEBUG(50, "B2Socket created (m_sock = " << m_sock << ")");

  return c_Success;
}

/// @brief Binding the socket created to specific port
/// @param port Port number to bind the socket
/// @return c_Success Port binding success
/// @return c_InitFailed Port binding failed
EStatus B2Socket::bind(const int port)
{
  if (!is_valid()) {
    B2ERROR("Socket file descriptor doesn't exist!");
    return c_InitFailed;
  }

  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  m_addr.sin_port = htons(port);

  int bind_return = ::bind(m_sock, (struct sockaddr*) & m_addr, sizeof(m_addr));

  if (bind_return == -1) {
    B2ERROR("Binding failed (code = " << errno << ")");
    return c_InitFailed;
  }

  B2DEBUG(50, "B2Socket bound (bind_return = " << bind_return << ")");

  return c_Success;
}

/// @brief Listen to the socket
/// @return c_Success Listening success
/// @return c_InitFailed Listening failed
EStatus B2Socket::listen() const
{
  if (!is_valid())
    return c_InitFailed;

  int listen_return = ::listen(m_sock, m_maxcons);

  if (listen_return == -1)
    return c_InitFailed;

  B2DEBUG(50, "Starting to listen through m_sock = " << m_sock << " with max connection of " << m_maxcons << ")");

  return c_Success;
}

/// @brief Accept the socket
/// @return 1 for success
/// @return 0 for no accepted sockets
/// @return -1 for error
EStatus B2Socket::accept(B2Socket& new_socket) const
{
  int addr_length = sizeof(m_addr);
  new_socket.m_sock = ::accept(m_sock, (sockaddr*) & m_addr, (socklen_t*) & addr_length);

  if (new_socket.m_sock < 0)
    return c_FuncError;
  else if (new_socket.m_sock == 0)
    return c_FuncError;
  else
    return c_Success;
}

/// @brief Send string through the socket
/// @param s String to be sent
/// @return c_Success Sending success
/// @return c_FuncError Sending failed
EStatus B2Socket::send(const std::string s) const
{
  int status = ::send(m_sock, s.c_str(), s.size(), MSG_NOSIGNAL);
  if (status == -1)
    return c_FuncError;
  else
    return c_Success;
}

/// @brief Send NodeInfo object through the socket (for control)
///        This implies the simple object serialization
/// @param nodeinfo NodeInfo object
/// @return c_Success Sending success
/// @return c_InitFailed Sending failed
EStatus B2Socket::send(NodeInfo* nodeinfo) const
{
  const std::string serializedNodeInfo = nodeinfo->serializedNodeInfo();
  return  send(serializedNodeInfo);
}

/// @brief Receive string from the socket
/// @param s String buffer to take data
/// @return 0 for error
/// @return otherwise Receiving status
int B2Socket::recv(std::string& s) const
{
  char buf[m_maxrecv + 1];

  s = "";

  memset(buf, 0, m_maxrecv + 1);

  int status = ::recv(m_sock, buf, m_maxrecv, 0);

  if (status == -1) {
    return 0;
  } else if (status == 0) {
    return 0;
  } else {
    s = buf;
    return status;
  }
}

/// @brief Connect to a host
/// @param host Host IP address to connect
/// @param port Port number to use for the communication
/// @return c_Success Connection success
/// @return c_FuncError Connection failed
EStatus B2Socket::connect(const std::string host, const int port)
{
  if (!is_valid()) {
    B2ERROR("B2SOCKET: Invalid");
    return c_FuncError;
  }

  m_addr.sin_family = AF_INET;
  m_addr.sin_port = htons(port);

  int status = inet_pton(AF_INET, host.c_str(), &m_addr.sin_addr);

  if (errno == EAFNOSUPPORT) {
    B2ERROR("B2SOCKET: EAFNOSUPPORT");
    return c_FuncError;
  }

  status = ::connect(m_sock, (sockaddr*) & m_addr, sizeof(m_addr));

  if (status == 0)
    return c_Success;
  else {
    if (errno != EALREADY && errno != EISCONN) {
      B2ERROR("Connection failed");
      return c_FuncError;
    } else
      return c_Success;
  }
}

/// @brief Validation check
/// @return c_Success Valid socket
/// @return c_FuncError Invalid socket
EStatus B2Socket::is_valid() const
{
  if (m_sock != -1)
    return c_Success;
  else
    return c_FuncError;
}

/// @brief Set non blocking status
/// @param b Blocking mode
void B2Socket::set_non_blocking(const bool b)
{
  int opts;

  opts = fcntl(m_sock, F_GETFL);

  if (opts < 0)
    return;

  if (b)
    opts = (opts | O_NONBLOCK);
  else
    opts = (opts & ~O_NONBLOCK);

  fcntl(m_sock, F_SETFL, opts);
}

/*
void B2Socket::displayError (std::string caller, int errno) {
  // Related to socket () function
  if (caller == "socket") {
    if (errno == EACCES) {
      B2ERROR ("Permission to create a socket denied");
    }
    else if (errno == EAFNOSUPPORT) {
      B2ERROR ("Address family is not supported");
    }
    else if (errno == EINVAL)
      B2ERROR ("Unknown protocol or protocol family not available");
    else if (errno == EMFILE)
      B2ERROR ("Process file table overflow");
    else if (errno == ENFILE)
      B2ERROR ("Supported number of open files exceeds the limit of the system");
    else if (errno == ENOBUFS)
      B2ERROR ("Not enough memory");
    else if (errno == ENOMEM)
      B2ERROR ("Not enough memory");
    else if (errno == EPROTONOSUPPORT)
      B2ERROR ("Protocol type is not supported in this domain");
    else
      B2ERROR ("B2SOCKET: Unknown error (" << errno << ")");
  }

  // Related to bind () function
  if (caller == "bind") {
    if (errno == EACCES)
      B2ERROR ("Permission to create a socket denied");
    else if (errno == EADDRINUSE)
      B2ERROR ("The given address is already in use");
    else {
      B2ERROR ("B2SOCKET: Unknown error (" << errno << ")");
      return false;
    }
  }

  // Related to connect () function
  if (caller == "connect") {
    if (errno == EACCES) {
      B2ERROR ("B2SOCKET: Write permission denied (EACCES)");
      return false;
    }
    else if (errno == EPERM) {
      B2ERROR ("B2SOCKET: Permission denied (EPERM)");
      return false;
    }
    else if (errno == EADDRINUSE) {
      B2ERROR ("B2SOCKET: Local address is already in use (EADDRINUSE)");
      return false;
    }
    else if (errno == EAFNOSUPPORT) {
      B2ERROR ("B2SOCKET: Incorrect address family (EAFNOSUPPORT)");
      return false;
    }
    else if (errno == EAGAIN) {
      B2ERROR ("B2SOCKET: No more free local ports or insufficient entries in the routing cache (EAGAIN)");
      return false;
    }
    else if (errno == EALREADY) {
      //B2INFO ("B2SOCKET: Connection exists (EALREADY)");
      return true;
    }
    else if (errno == EBADF) {
      B2ERROR ("B2SOCKET: Invalid describer has been passed to socket (EBADF)");
      return false;
    }
    else if (errno == ECONNREFUSED) {
      B2ERROR ("B2SOCKET: Connection refused (ECONNREFUSED)");
      return false;
    }
    else if (errno == EFAULT) {
      B2ERROR ("B2SOCKET: Socket structure address can't be reachable (EFAULT)");
      return false;
    }
    else if (errno == EINPROGRESS) {
      B2ERROR ("B2SOCKET: Socket is nonblocking (EINPROGRESS)");
      return false;
    }
    else if (errno == EINTR) {
      B2ERROR ("B2SOCKET: System call was interrupted by a signal (EINTR)");
      return false;
    }
    else if (errno == EISCONN) {
      //B2INFO ("B2SOCKET: Already connected (EISCONN)");
      return true;
    }
    else if (errno == ENETUNREACH) {
      B2ERROR ("B2SOCKET: Network is unreachable (ENETUNREACH)");
      return false;
    }
    else if (errno == ENOTSOCK) {
      B2ERROR ("B2SOCKET: File descriptor is not associated with a socket (ENOTSOCK)");
      return false;
    }
    else if (errno == ETIMEDOUT) {
      B2ERROR ("B2SOCKET: Connection timed out (ETIMEOUT)");
      return false;
    }
    else {
      B2ERROR ("B2SOCKET: Unknown error (" << errno << ")");
      return false;
    }
  }
}
*/
