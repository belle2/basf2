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
  try {
    NSMMessage msg;
    NSMMessage msg_out;
    NSMCommand cmd;
    while (true) {
      m_reader.readObject(msg);
      cmd = msg.getRequestName();
      LogFile::debug("%s %s", msg.getRequestName(), cmd.getLabel());
      if (cmd == Enum::UNKNOWN) {
        LogFile::warning("Unknown request : %s", msg.getRequestName());
      } else if (cmd == NSMCommand::DBGET) {
        DB2NSM(m_db).set(msg, msg_out);
        sendMessage(msg_out);
      } else if (cmd == NSMCommand::DBSET) {
        DB2NSM(m_db).get(msg);
      } else if (cmd == NSMCommand::NSMGET) {
        StringList argv = StringUtil::split(msg.getData(), ' ', 2);
        try {
          int revision = (msg.getNParams() > 0) ? msg.getParam(0) : 0;
          NSMData& data(m_bridge->getCallback()->getData(argv[0], argv[1], revision));
          if (data.isAvailable()) {
            msg_out.setRequestName(NSMCommand::NSMSET);
            msg_out.setData(data);
            sendMessage(msg_out);
          }
        } catch (const NSMHandlerException& e) {
          LogFile::error(e.what());
          msg_out.setRequestName(NSMCommand::LOG);
          msg_out.setData(DAQLogMessage("NSM2Socket", LogFile::WARNING,
                                        StringUtil::form("NSM data (%s) is not ready for read",
                                                         argv[0].c_str())));
          sendMessage(msg_out);
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

