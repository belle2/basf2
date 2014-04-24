#include "daq/slc/apps/envmonitord/DataSender.h"
#include "daq/slc/apps/envmonitord/EnvMonitorMaster.h"

#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/LogFile.h>

#include <cstdlib>
#include <vector>

using namespace Belle2;

DataSender::~DataSender()
{
  m_socket.close();
  LogFile::debug("GUI disconnected.");
}

void DataSender::run()
{
  TCPSocketWriter socket_writer(m_socket);
  TCPSocketReader socket_reader(m_socket);
  std::vector<NSMData>& data_v(m_master.getData());
  m_master.lock();
  socket_writer.writeInt(data_v.size());
  m_master.unlock();
  while (true) {
    m_master.lock();
    try {
      for (size_t i = 0; i < data_v.size(); i++) {
        if (data_v[i].isAvailable()) {
          socket_writer.writeInt(i);
          socket_writer.writeObject(data_v[i]);
        }
      }
    } catch (const IOException& e) {
      m_master.unlock();
      return;
    }
    m_master.wait();
    m_master.unlock();
  }
}

