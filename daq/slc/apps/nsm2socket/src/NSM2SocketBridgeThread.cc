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
  m_bridge->add(this);
}

NSM2SocketBridgeThread::~NSM2SocketBridgeThread() throw()
{
  m_bridge->remove(this);
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
  try {
    NSMMessage msg;
    while (true) {
      NSMMessage msg_out;
      msg.readObject(m_reader);
      NSMCommand cmd(msg.getRequestName());
      if (cmd == NSMCommand::DBGET) {
        DB2NSM(m_db).set(msg, msg_out);
        sendMessage(msg_out);
      } else if (cmd == NSMCommand::DBSET) {
        DB2NSM(m_db).get(msg);
      } else if (cmd == NSMCommand::NSMGET) {
        NSMDataList& data_v(m_bridge->getCallback()->getDataList());
        for (NSMDataList::iterator it = data_v.begin();
             it != data_v.end(); it++) {
          NSMData& data(*it);
          if (data.isAvailable()) {
            msg_out.init();
            msg_out.setRequestName(NSMCommand::NSMSET);
            msg_out.setData(data);
            sendMessage(msg_out);
          }
        }
      } else if (cmd == NSMCommand::NSMSET) {
      } else {
        try {
          m_bridge->getCallback()->sendRequest(msg);
        } catch (const NSMHandlerException& e) {
          LogFile::error("NSM error");
        }
      }
    }
  } catch (const IOException& e) {
    LogFile::error("%s", e.what());
    m_socket.close();
  }
}

