#include <daq/hlt/B2Socket.h>

using namespace Belle2;

B2Socket::B2Socket()
{
  m_maxHosts = gMaxHosts;
  m_maxConnections = gMaxConnections;
  m_maxReceives = gMaxReceives;

  m_socket = -1;

  memset(&m_socketAddress, 0, sizeof(m_socketAddress));
}

B2Socket::~B2Socket()
{
  B2INFO("\x1b[34m[B2Socket] Closing connection...\x1b[0m");
  close(m_socket);
}

EHLTStatus B2Socket::create()
{
  m_socket = ::socket(AF_INET, SOCK_STREAM, 0);

  if (!isValid()) {
    B2ERROR("\x1b[31m[B2Socket] Socket creation failed! (errno="
            << errno << ")\x1b[0m");
    return c_InitFailed;
  }

  int on = 1;

  if (::setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) == -1)
    return c_InitFailed;

  B2INFO("\x1b[34m[B2Socket] Socket created (m_socket=" << m_socket << ")\x1b[0m");

  return c_Success;
}

EHLTStatus B2Socket::bind(const unsigned int port)
{
  if (!isValid()) {
    B2ERROR("\x1b[31m[B2Socket] Socket doesn't exist!\x1b[0m");
    return c_InitFailed;
  }

  m_socketAddress.sin_family = AF_INET;
  m_socketAddress.sin_addr.s_addr = INADDR_ANY;
  m_socketAddress.sin_port = ::htons(port);

  int bindReturn = ::bind(m_socket, (struct sockaddr*)&m_socketAddress, sizeof(m_socketAddress));

  if (bindReturn == -1) {
    B2ERROR("\x1b[31m[B2Socket] Socket " << m_socket << " binding failed with code=" << errno << "\x1b[0m");
    return c_InitFailed;
  }

  B2INFO("\x1b[34m[B2Socket] Socket " << m_socket << " has been bound through "
         << port << "\x1b[0m");

  return c_Success;
}

EHLTStatus B2Socket::connect(const std::string destination, const int port)
{
  if (!isValid()) {
    B2ERROR("\x1b[31m[B2Socket] Socket doesn't exist!\x1b[0m");
    return c_InitFailed;
  }

  setNonBlocking(true);

  m_socketAddress.sin_family = AF_INET;
  m_socketAddress.sin_port = ::htons(port);

  int connectReturn = ::inet_pton(AF_INET, destination.c_str(), &m_socketAddress.sin_addr);

  connectReturn = ::connect(m_socket, (sockaddr*)&m_socketAddress, sizeof(m_socketAddress));

  setNonBlocking(false);

  if (!connectReturn) {
    return c_Success;
  } else {
    B2ERROR("\x1b[31m[B2Socket] Connection failed to "
            << destination << " (errno=" << errno << ")\x1b[0m");
    return c_InitFailed;
  }
}

EHLTStatus B2Socket::listen()
{
  if (!isValid()) {
    B2ERROR("\x1b[31m[B2Socket] Socket doesn't exist!\x1b[0m");
    return c_InitFailed;
  }

  int listenReturn = ::listen(m_socket, m_maxConnections);

  if (listenReturn == -1) {
    B2ERROR("\x1b[31m[B2Socket] Listen failed\x1b[0m");
    return c_InitFailed;
  } else
    return c_Success;
}

EHLTStatus B2Socket::accept(int& newSocket)
{
  B2INFO("\x1b[34m[B2Socket] Accepting a new socket...\x1b[0m");
  int addressLength = sizeof(m_socketAddress);
  int status = ::accept(m_socket, (struct sockaddr*)&m_socketAddress, (socklen_t*)&addressLength);

  if (status == -1) {
    B2ERROR("\x1b[31m[B2Socket] Can't find proper socket!\x1b[0m");
    return c_InitFailed;
  } else {
    newSocket = status;
    B2INFO("\x1b[34m[B2Socket] Accept new socket = "
           << newSocket << "\x1b[0m");
    return c_Success;
  }
}

EHLTStatus B2Socket::send(const std::string data, int& size)
{
  int status = ::send(m_socket, data.c_str(), data.size(), 0);

  if (status == -1) {
    B2ERROR("\x1b[31m[B2Socket] Sending data " << data << " failed\x1b[0m");
    return c_FuncError;
  } else {
    size = status;
    return c_Success;
  }
}

EHLTStatus B2Socket::receive(int newSocket, std::string& data, int& size)
{
  char buffer[m_maxReceives + 1];
  memset(buffer, 0, m_maxReceives + 1);

  int status = ::recv(newSocket, buffer, m_maxReceives, 0);

  if (status == -1) {
    B2ERROR("\x1b[31m[B2Socket] Data receiving failed\x1b[0m");
    return c_FuncError;
  } else if (status == 0) {
    return c_FuncError;
  } else {
    std::string tempData(buffer);

    if (tempData == "Terminate")
      return c_TermCalled;

    data = tempData;
    size = status;

    return c_Success;
  }

  return c_Success;
}

bool B2Socket::isValid()
{
  if (m_socket == -1)
    return false;
  else
    return true;
}

void B2Socket::setNonBlocking(const bool flag)
{
  int option = ::fcntl(m_socket, F_GETFL);

  if (option < 0)
    return;

  if (flag)
    option = (option | O_NONBLOCK);
  else
    option = (option & ~O_NONBLOCK);

  ::fcntl(m_socket, F_SETFL, option);
}

