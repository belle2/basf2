#include "daq/slc/apps/runcontrold/RCGUIAcceptor.h"

#include "daq/slc/apps/runcontrold/RCGUICommunicator.h"
#include "daq/slc/apps/runcontrold/RCMasterCallback.h"
#include "daq/slc/apps/runcontrold/RCMaster.h"

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketReader.h>

#include <daq/slc/base/Debugger.h>

using namespace Belle2;

void RCGUIAcceptor::run()
{
  TCPServerSocket server_socket(_ip, _port);
  server_socket.open();
  while (true) {
    Belle2::debug("Waiting for new connection..");
    TCPSocket socket = server_socket.accept();
    Belle2::debug("Accepted new connection..");
    TCPSocketReader reader(socket);
    std::string name = reader.readString();
    if (name == "GUI") {
      TCPSocketWriter writer(socket);
      RCMaster* master = _callback->getMaster();
      master->lock();
      writer.writeString(master->getNode()->getName());
      master->getData()->writeObject(writer);
      master->getStatus()->writeObject(writer);
      master->getConfig()->writeObject(writer);
      master->unlock();
      RCGUICommunicator* comm = new RCGUICommunicator(socket, _callback);
      comm->run();
      delete comm;
    }
  }
}

