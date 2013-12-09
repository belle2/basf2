#include "daq/slc/apps/runcontrold/RCClientAcceptor.h"

#include "daq/slc/apps/runcontrold/RCMaster.h"

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/TCPSocketWriter.h>

using namespace Belle2;

void RCClientAcceptor::run()
{
  TCPServerSocket server_socket(_ip, _port);
  server_socket.open();
  while (true) {
    TCPSocket socket = server_socket.accept();
    TCPSocketReader reader(socket);
    std::string name = reader.readString();
    _master->lock();
    DataObject* mdata = _master->getData();
    TCPSocketWriter writer(socket);
    if (mdata->hasObject(name)) {
      DataObject* data = mdata->getObject(name);
      try {
        if (data != NULL) {
          data->writeObject(writer);
        }
      } catch (const IOException& e) {}
    } else {
      try {
        DataObject().writeObject(writer);
      } catch (const IOException& e) {}
    }
    socket.close();
    _master->unlock();
  }
}

