#include "daq/slc/apps/nsm2socket/NSM2SocketBridgeThread.h"

#include "daq/slc/apps/nsm2socket/NSM2SocketBridge.h"
#include "daq/slc/apps/nsm2socket/NSM2SocketCallback.h"
#include "daq/slc/apps/nsm2socket/DB2NSM.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

NSM2SocketBridgeThread::NSM2SocketBridgeThread(const TCPSocket& socket,
                                               NSM2SocketBridge* bridge,
                                               DBInterface* db)
  : m_bridge(bridge), m_socket(socket), m_db(db),
    m_reader(socket), m_writer(socket)
{
}

NSM2SocketBridgeThread::~NSM2SocketBridgeThread() throw()
{
}

void NSM2SocketBridgeThread::sendMessage(NSMMessage& msg) throw()
{
  m_mutex.lock();
  try {
    m_writer.writeObject(msg);
  } catch (const IOException& e) {
    LogFile::error("Failed to send state");
    m_socket.close();
  }
  m_mutex.unlock();
}

void NSM2SocketBridgeThread::run() throw()
{
}

