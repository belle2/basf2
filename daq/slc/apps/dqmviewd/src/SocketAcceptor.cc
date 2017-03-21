#include <daq/slc/apps/dqmviewd/SocketAcceptor.h>

#include <daq/slc/apps/dqmviewd/DQMViewCallback.h>

#include <daq/slc/apps/dqmviewd/HistSender.h>

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/LogFile.h>

using namespace Belle2;

void SocketAcceptor::run()
{
  TCPServerSocket server_socket(m_ip, m_port);
  server_socket.open();
  LogFile::debug("open socket (%s:%d)", m_ip.c_str(), m_port);
  while (true) {
    TCPSocket socket = server_socket.accept();
    LogFile::debug("new connection accepted (%s:%d)", m_ip.c_str(), m_port);
    m_callback->addSender(HistSender(socket, m_callback));
  }
}
