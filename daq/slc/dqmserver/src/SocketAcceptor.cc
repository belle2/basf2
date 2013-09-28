#include "SocketAcceptor.hh"

#include "HistoServer.hh"
#include "PackageSender.hh"

#include <system/TCPServerSocket.hh>
#include <system/PThread.hh>

using namespace B2DQM;

void SocketAcceptor::run()
{
  B2DAQ::TCPServerSocket server_socket(_ip, 50100);
  server_socket.open();
  while (true) {
    B2DAQ::TCPSocket socket = server_socket.accept();
    B2DAQ::PThread(new PackageSender(_server, socket));
  }
}
