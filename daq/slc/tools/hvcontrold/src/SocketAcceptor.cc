#include "SocketAcceptor.h"

#include "HVControlMaster.h"
//#include "HVParamProvider.h"

#include <system/TCPServerSocket.h>
#include <system/PThread.h>

using namespace Belle2;

void SocketAcceptor::run()
{
  TCPServerSocket server_socket(_ip, _port);
  server_socket.open();
  while (true) {
    TCPSocket socket = server_socket.accept();
    _master->accept(socket);
    socket.close();
    //PThread(new HVParamProvider(socket, _master));
  }
}
