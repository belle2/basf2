#include "daq/slc/apps/nsm2socket/NSM2SocketBridge.h"

#include "daq/slc/apps/nsm2socket/NSM2SocketCallback.h"

#include <daq/slc/database/ConfigInfoTable.h>
#include <daq/slc/database/ConfigObjectTable.h>
#include <daq/slc/database/LoggerObjectTable.h>
#include <daq/slc/database/RunNumberInfoTable.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/runcontrol/RCCommand.h>
#include <daq/slc/hvcontrol/HVCommand.h>

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

bool NSM2SocketBridge::sendMessage(const NSMMessage& msg) throw()
{
  try {
    m_writer.writeObject(msg);
    return true;
  } catch (const IOException& e) {
    LogFile::warning("Connection failed for writing");
  }
  return false;
}

bool NSM2SocketBridge::recieveMessage(NSMMessage& msg) throw()
{
  try {
    msg.init();
    m_reader.readObject(msg);
    return true;
  } catch (const IOException& e) {
    LogFile::warning("Connection failed for reading");
  }
  return false;
}

void NSM2SocketBridge::run() throw()
{
  try {
    m_server_socket.open();
  } catch (const IOException& e) {
    LogFile::error("Failed to open socket");
  }
  TCPSocket socket;
  while (true) {
    socket = m_server_socket.accept();
    LogFile::debug("Accepted new connection");
    m_writer = TCPSocketWriter(socket);
    m_reader = TCPSocketReader(socket);
    NSMMessage msg;
    while (true) {
      NSMMessage msg_out(m_callback->getNode().getName());
      if (!recieveMessage(msg)) {
        socket.close();
        break;
      }
      const std::string reqname = msg.getRequestName();
      NSMCommand cmd(reqname.c_str());
      if (cmd == NSMCommand::DBGET) {
        try {
          m_db->connect();
          StringList str_v = StringUtil::split(msg.getData(), ' ', 5);
          std::string nodename = str_v[1];
          std::string configname = str_v[2];
          if (msg.getNParams() > 0 && msg.getParam(0) > 0) {
            configname = ConfigInfoTable(m_db).get(msg.getParam(0)).getName();
          }
          ConfigObject obj = ConfigObjectTable(m_db).get(configname, nodename);
          msg_out.setRequestName(NSMCommand::DBSET);
          msg_out.setData(obj);
          sendMessage(msg_out);
          ConfigObjectTable(m_db).get(configname, nodename);
        } catch (const DBHandlerException& e) {
          LogFile::error(e.what());
        }
        m_db->close();
      } else if (cmd == NSMCommand::DBSET) {
        try {
          ConfigObject obj;
          if (msg.getLength() > 0) {
            msg.getData(obj);
            m_db->connect();
            ConfigObjectTable(m_db).add(obj);
          }
        } catch (const DBHandlerException& e) {
          LogFile::error(e.what());
        }
        m_db->close();
      } else if (cmd == NSMCommand::NSMGET) {
        StringList argv = StringUtil::split(msg.getData(), ' ', 2);
        try {
          int revision = (msg.getNParams() > 0) ? msg.getParam(0) : 0;
          NSMData& data(m_callback->getData(argv[0], argv[1], revision));
          msg_out.setRequestName(NSMCommand::NSMSET);
          msg_out.setData(data);
        } catch (const NSMHandlerException& e) {
          LogFile::error(e.what());
          msg_out.setRequestName(NSMCommand::LOG);
          msg_out.setData(DAQLogMessage("NSM2Socket", LogFile::WARNING,
                                        StringUtil::form("NSM data (%s) is not ready for read",
                                                         argv[0].c_str())));
        }
        sendMessage(msg_out);
      } else if (HVCommand(reqname) != Enum::UNKNOWN ||
                 RCCommand(reqname) != Enum::UNKNOWN) {
        m_callback->getCommunicator()->sendRequest(msg);
      } else {
        LogFile::warning("Unknown request : %s", reqname.c_str());
      }
    }
  }
}

