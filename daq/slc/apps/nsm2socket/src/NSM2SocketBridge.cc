#include "daq/slc/apps/nsm2socket/NSM2SocketBridge.h"

#include "daq/slc/apps/nsm2socket/NSM2SocketCallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/database/DBObjectLoader.h>
#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/runcontrol/RCCommand.h>
#include <daq/slc/hvcontrol/HVCommand.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <unistd.h>
#include <sstream>

using namespace Belle2;

NSM2SocketBridge::NSM2SocketBridge(const TCPSocket& socket)
  : m_callback(NULL), m_db(NULL), m_socket(socket),
    m_writer(socket), m_reader(socket)
{
}

NSM2SocketBridge::~NSM2SocketBridge() throw()
{
}

void NSM2SocketBridge::run() throw()
{
  LogFile::close();
  try {
    NSMNode node(m_reader.readString());
    std::string host = m_reader.readString();
    int port = m_reader.readInt();
    LogFile::open("nsm2socket." + StringUtil::tolower(node.getName()));
    m_callback = new NSM2SocketCallback(node);
    m_callback->setBridge(this);
    PThread(new NSMNodeDaemon(m_callback, host, port));
    ConfigFile config("slowcontrol");
    m_db = new PostgreSQLInterface(config.get("database.host"),
                                   config.get("database.dbname"),
                                   config.get("database.user"),
                                   config.get("database.password"),
                                   config.getInt("database.port"));
    while (true) {
      NSMMessage msg;
      m_reader.readObject(msg);
      NSMCommand cmd(msg.getRequestName());
      if (cmd == NSMCommand::DBGET || cmd == NSMCommand::DBLISTGET) {
        if (msg.getLength() > 0) {
          StringList s = StringUtil::split(msg.getData(), '/');
          if (s.size() >= 2) {
            const std::string table = s[0];
            const std::string config = s[1];
            StringList ss = StringUtil::split(s[1], '@');
            NSMNode node(ss[0]);
            if (cmd == NSMCommand::DBGET) {
              dbget(table, config, node);
            } else if (cmd == NSMCommand::DBLISTGET) {
              dblistget(table, node, (ss.size() > 1) ? ss[1] : "");
            }
          }
        }
      } else if (cmd == NSMCommand::DBSET) {
        if (msg.getLength() > 0) {
          DBObject obj;
          m_reader.readObject(obj);
          dbset(msg.getData(), obj);
        }
      } else if (cmd == NSMCommand::DATAGET) {
        if (msg.getLength() > 0) {
          StringList argv = StringUtil::split(msg.getData(), ' ');
          if (argv.size() >= 2) {
            int revision = (msg.getNParams() > 0) ? msg.getParam(0) : -1;
            nsmdataget(revision, argv[0], argv[1]);
          }
        }
      }  else if (cmd == NSMCommand::VSET) {
        NSMNode node(msg.getNodeName());
        NSMVar var;
        m_reader.readObject(var);
        m_callback->send(NSMMessage(node, var));
      } else {
        m_callback->send(msg);
      }
    }
  } catch (const IOException& e) {
    LogFile::warning("Connection failed for reading");
    m_socket.close();
  }
}

void NSM2SocketBridge::dbget(const std::string& table,
                             const std::string& config,
                             const NSMNode& node)
{
  DBInterface& db(getDB());
  try {
    DBObject obj(DBObjectLoader::load(db, table, config, false));
    send(NSMMessage(node, NSMCommand::DBSET), obj);
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
  db.close();
}

void NSM2SocketBridge::dblistget(const std::string& table,
                                 const NSMNode& node,
                                 const std::string& grep)
{
  DBInterface& db(getDB());
  try {
    StringList ss = DBObjectLoader::getDBlist(db, table, node.getName(), grep);
    send(NSMMessage(NSMNode(node), NSMCommand::DBLISTSET,
                    ss.size(), StringUtil::join(ss, "\n")));
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
  db.close();
}

void NSM2SocketBridge::dbset(const std::string& table,
                             const DBObject& obj)
{
  DBInterface& db(getDB());
  try {
    DBObjectLoader::createDB(db, table, obj);
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
  db.close();
}

void NSM2SocketBridge::nsmdataget(int revision,
                                  const std::string& name,
                                  const std::string& format)
{
  const NSMNode& node(m_callback->getNode());
  try {
    NSMData& data(m_callback->getData(name, format, revision));
    if (data.isAvailable()) {
      send(NSMMessage(node, NSMCommand::DATASET), data);
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
    std::string message =
      StringUtil::form("NSM data (%s) is not ready for read: %s",
                       name.c_str(), e.what());
    send(NSMMessage(node, DAQLogMessage(node.getName(),
                                        LogFile::WARNING, message)));
  }
}

bool NSM2SocketBridge::send(const NSMMessage& msg) throw()
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

bool NSM2SocketBridge::send(const NSMMessage& msg,
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
    m_mutex.unlock();
  }
  return false;
}
