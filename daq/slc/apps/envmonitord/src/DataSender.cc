#include "daq/slc/apps/envmonitord/DataSender.h"
#include "daq/slc/apps/envmonitord/EnvMonitorMaster.h"

#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Debugger.h>

#include <cstdlib>
#include <vector>

using namespace Belle2;

DataSender::~DataSender()
{
  _socket.close();
  LogFile::debug("GUI disconnected.");
}

void DataSender::run()
{
  TCPSocketWriter socket_writer(_socket);
  TCPSocketReader socket_reader(_socket);
  std::vector<NSMData*>& data_v(_master->getData());
  _master->lock();
  socket_writer.writeInt(data_v.size());
  _master->unlock();
  while (true) {
    _master->lock();
    try {
      for (size_t i = 0; i < data_v.size(); i++) {
        if (data_v[i]->isAvailable()) {
          socket_writer.writeInt(i);
          socket_writer.writeObject(*data_v[i]);
        }
      }
    } catch (const IOException& e) {
      _master->unlock();
      return;
    }
    _master->wait();
    _master->unlock();
  }
}

