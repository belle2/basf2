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

#include <unistd.h>
#include <sstream>

using namespace Belle2;

NSM2SocketBridge::NSM2SocketBridge(const TCPSocket& socket,
                                   NSM2SocketCallback* callback,
                                   DBInterface* db)
  : m_callback(callback), m_db(db), m_socket(socket),
    m_writer(socket), m_reader(socket)
{
  m_callback->setBridge(this);
}

NSM2SocketBridge::~NSM2SocketBridge() throw()
{
}

bool NSM2SocketBridge::sendMessage(const NSMMessage& msg) throw()
{
  m_mutex.lock();
  try {
    m_writer.writeObject(msg);
    m_mutex.unlock();
    return true;
  } catch (const IOException& e) {
    LogFile::warning("Connection failed for writing : %s", e.what());
    m_socket.close();
  }
  m_mutex.unlock();
  return false;
}

bool NSM2SocketBridge::sendMessage(const NSMMessage& msg,
                                   const Serializable& obj) throw()
{
  m_mutex.lock();
  try {
    m_writer.writeObject(msg);
    m_writer.writeObject(obj);
    m_mutex.unlock();
    return true;
  } catch (const IOException& e) {
    LogFile::warning("Connection failed for writing");
    m_socket.close();
  }
  m_mutex.unlock();
  return false;
}

bool NSM2SocketBridge::sendLog(const DAQLogMessage& log) throw()
{
  NSMMessage msg(m_callback->getNode());
  msg.setRequestName(NSMCommand::LOG);
  msg.setNParams(2);
  msg.setParam(0, (int)log.getPriority());
  msg.setParam(1, log.getDateInt());
  msg.setData(log.getNodeName() + "\n" + log.getMessage());
  return sendMessage(msg);
}

bool NSM2SocketBridge::sendError(const ERRORNo& eno,
                                 const std::string& nodename,
                                 const std::string& message) throw()
{
  NSMNode node(nodename);
  NSMMessage msg(node);
  msg.setRequestName(NSMCommand::ERROR);
  msg.setNParams(1);
  msg.setParam(0, eno.getId());
  msg.setData(message);
  return sendMessage(msg);
}

bool NSM2SocketBridge::recieveMessage(NSMMessage& msg) throw()
{
  try {
    msg.init();
    m_reader.readObject(msg);
    return true;
  } catch (const IOException& e) {
    LogFile::warning("Connection failed for reading");
    m_socket.close();
  }
  return false;
}

void NSM2SocketBridge::run() throw()
{
  sleep(5);
  while (true) {
    NSMMessage msg_out(m_callback->getNode());
    NSMMessage msg;
    if (!recieveMessage(msg)) {
      return;
    }
    const std::string reqname = msg.getRequestName();
    NSMCommand cmd(reqname.c_str());
    if (cmd == NSMCommand::DBGET) {
      std::string nodename;
      try {
        m_db->connect();
        StringList str_v = StringUtil::split(msg.getData(), ' ', 5);
        nodename = str_v[1];
        std::string configname = str_v[2];
        if (msg.getNParams() > 0 && msg.getParam(0) > 0) {
          configname = ConfigInfoTable(m_db).get(msg.getParam(0)).getName();
        }
        ConfigObject obj = ConfigObjectTable(m_db).get(configname, nodename);
        msg_out.setNodeName(nodename);
        msg_out.setRequestName(NSMCommand::DBSET);
        sendMessage(msg_out, obj);
        ConfigObjectTable(m_db).get(configname, nodename);
      } catch (const DBHandlerException& e) {
        LogFile::error(e.what());
        sendError(ERRORNo::DATABASE, nodename, "Failed to read DB");
      }
      m_db->close();
    } else if (cmd == NSMCommand::DBSET) {
      ConfigObject obj;
      try {
        if (msg.getLength() > 0) {
          m_reader.readObject(obj);
          m_db->connect();
          ConfigObjectTable(m_db).addAll(obj, true);
        }
      } catch (const DBHandlerException& e) {
        LogFile::error(e.what());
        sendError(ERRORNo::DATABASE, obj.getNode(), "Failed to write DB");
      }
      m_db->close();
    } else if (cmd == NSMCommand::LISTGET) {
      std::string nodename;
      try {
        m_db->connect();
        nodename = msg.getData();
        ConfigInfoList info_v = ConfigInfoTable(m_db).getList(nodename);
        m_db->close();
        std::stringstream ss;
        for (size_t i = 0; i < info_v.size(); i++) {
          ss << info_v[i].getName() << "\n";
        }
        msg_out.setNodeName(nodename);
        msg_out.setRequestName(NSMCommand::LISTSET);
        msg_out.setNParams(1);
        msg_out.setParam(0, info_v.size());
        msg_out.setData(ss.str());
        sendMessage(msg_out);
      } catch (const DBHandlerException& e) {
        m_db->close();
        LogFile::error(e.what());
        sendError(ERRORNo::DATABASE, nodename, "Failed to read DB list");
      }
    } else if (cmd == NSMCommand::NSMGET) {
      StringList argv = StringUtil::split(msg.getData(), ' ', 2);
      try {
        int revision = (msg.getNParams() > 0) ? msg.getParam(0) : 0;
        NSMData& data(m_callback->getData(argv[0], argv[1], revision));
        if (data.isAvailable()) {
          msg_out.setNodeName(data.getName());
          msg_out.setRequestName(NSMCommand::NSMSET);
          sendMessage(msg_out, data);
        }
      } catch (const NSMHandlerException& e) {
        LogFile::error(e.what());
        std::string message =
          StringUtil::form("NSM data (%s) is not ready for read: %s",
                           argv[0].c_str(), e.what());
        sendLog(DAQLogMessage(m_callback->getNode().getName(),
                              LogFile::WARNING, message));
        return;
      }
    } else if (NSMCommand(reqname) != Enum::UNKNOWN ||
               HVCommand(reqname) != Enum::UNKNOWN ||
               RCCommand(reqname) != Enum::UNKNOWN) {
      m_callback->sendRequest(msg);
    } else {
      LogFile::warning("Unknown request : %s", reqname.c_str());
    }
  }
}

