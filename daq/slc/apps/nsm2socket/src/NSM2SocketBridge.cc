#include "daq/slc/apps/nsm2socket/NSM2SocketBridge.h"

#include "daq/slc/apps/nsm2socket/NSM2SocketBridgeThread.h"
#include "daq/slc/apps/nsm2socket/NSM2SocketCallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

using namespace Belle2;

NSM2SocketBridge::NSM2SocketBridge(const std::string& host, int port,
                                   NSM2SocketCallback* callback,
                                   DBInterface* db)
  : m_callback(callback), m_db(db), m_server_socket(host, port)
{
  m_callback->setBridge(this);
}

NSM2SocketBridge::~NSM2SocketBridge() throw()
{
}

void NSM2SocketBridge::sendMessage(NSMMessage& msg) throw()
{
  m_mutex.lock();
  for (std::list<NSM2SocketBridgeThread*>::iterator it = m_thread_l.begin();
       it != m_thread_l.end(); it++) {
    (*it)->sendMessage(msg);
  }
  m_mutex.unlock();
}

void NSM2SocketBridge::add(NSM2SocketBridgeThread* thread)
{
  m_mutex.lock();
  m_thread_l.push_back(thread);
  m_mutex.unlock();
}

void NSM2SocketBridge::remove(NSM2SocketBridgeThread* thread)
{
  m_mutex.lock();
  m_thread_l.remove(thread);
  m_mutex.unlock();
}

void NSM2SocketBridge::run() throw()
{
  try {
    m_server_socket.open();
  } catch (const IOException& e) {
    LogFile::error("Failed to open server socket. Terminating..");
    return;
  }
  while (true) {
    try {
      TCPSocket socket = m_server_socket.accept();
      PThread(new NSM2SocketBridgeThread(socket, this, m_db));
    } catch (const IOException& e) {
      LogFile::error("Failed to accept connection. Terminating..");
      return;
    }
  }
}

