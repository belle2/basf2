#include "daq/slc/apps/runcontrold/RCGUIAcceptor.h"

#include "daq/slc/apps/runcontrold/RCGUICommunicator.h"
#include "daq/slc/apps/runcontrold/RCMasterCallback.h"
#include "daq/slc/apps/runcontrold/RCMaster.h"

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Debugger.h>

using namespace Belle2;

void RCGUIAcceptor::run()
{
  TCPServerSocket server_socket(_ip, _port);
  server_socket.open();
  while (true) {
    LogFile::debug("waiting for new connection..");
    TCPSocket socket = server_socket.accept();
    LogFile::debug("accepted new connection..");
    TCPSocketReader reader(socket);
    std::string name = reader.readString();
    if (name == "GUI" || name == "CUI") {
      TCPSocketWriter writer(socket);
      RCMaster* master = _callback->getMaster();
      master->lock();
      writer.writeString(master->getNode()->getName());
      master->getData()->writeObject(writer);
      master->getStatus()->writeObject(writer);
      master->getConfig()->writeObject(writer);
      writer.writeBool(!master->hasMasterCommunicator());
      master->unlock();
      RCGUICommunicator* comm = new RCGUICommunicator(socket, _callback);
      master->getNode()->setConnection(Connection::ONLINE);
      comm->sendState(master->getNode());
      PThread((RCGUICommunicator*)comm);
      //comm->run();
      //delete comm;
    }
    //socket.close();
  }
}

