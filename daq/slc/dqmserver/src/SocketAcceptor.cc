#include "SocketAcceptor.h"

#include "HistoServer.h"
#include "PackageSender.h"

#include <system/TCPServerSocket.h>
#include <system/PThread.h>

using namespace Belle2;

void SocketAcceptor::run()
{
  Belle2::TCPServerSocket server_socket(_ip, 50100);
  server_socket.open();
  while (true) {
    Belle2::TCPSocket socket = server_socket.accept();
    Belle2::PThread(new PackageSender(_server, socket));
  }
}
