#include "daq/slc/runcontrol/RCCallback.h"

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/TCPSocketWriter.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <cstring>
#include <cstdlib>

namespace Belle2 {
  class RCConfigHandler : public NSMVHandlerText {
  public:
    RCConfigHandler(RCCallback& callback,
                    const std::string& name, const std::string& val)
      : NSMVHandlerText(name, true, true, val), m_callback(callback) {}
    bool handleGetText(std::string& val)
    {
      const DBObject& obj(m_callback.getDBObject());
      val = obj.getName();
      return true;
    }
    bool handleSetText(const std::string& val)
    {
      RCState state(m_callback.getNode().getState());
      RCState tstate(RCCommand::CONFIGURE.nextTState());
      m_callback.setState(tstate);
      DBObject& obj(m_callback.getDBObject());
      obj.getName();
      try {
        m_callback.abort();
        m_callback.dbload(val.size(), val.c_str());
      } catch (const IOException& e) {
        throw (RCHandlerException(e.what()));
      }
      //set("rcconfig", m_obj.getName());
      m_callback.configure(obj);
      m_callback.setState(state);
      return true;
    }
  private:
    RCCallback& m_callback;
  };
}

using namespace Belle2;

RCCallback::RCCallback(int timeout) throw()
  : NSMCallback(timeout)
{
  reg(RCCommand::CONFIGURE);
  reg(RCCommand::BOOT);
  reg(RCCommand::LOAD);
  reg(RCCommand::START);
  reg(RCCommand::STOP);
  reg(RCCommand::RECOVER);
  reg(RCCommand::RESUME);
  reg(RCCommand::PAUSE);
  reg(RCCommand::ABORT);
  reg(NSMCommand::FATAL);
  m_auto = true;
  m_db = NULL;
  m_showall = true;
  m_expno = m_runno = 0;
}

void RCCallback::init(NSMCommunicator&) throw()
{
  NSMNode& node(getNode());
  reset();
  add(new NSMVHandlerText("dbtable", true, false, m_table));
  add(new NSMVHandlerText("rcstate", true, false, RCState::NOTREADY_S.getLabel()));
  setState(RCState::NOTREADY_S);
  add(new NSMVHandlerText("rcrequest", true, false, ""));
  if (m_file.size() == 0 && (m_table.size() == 0 || m_rcconfig.size() == 0)) {
    LogFile::notice("dbtable or rcconfig is empty (dbtable='%s', runtype='%s')",
                    m_table.c_str(), m_rcconfig.c_str());
  } else {
    m_rcconfig = node.getName() + "@RC:" + m_rcconfig;
    if (m_file.size() > 0) {
      m_obj = DBObjectLoader::load(m_file);
      m_obj.print(m_showall);
    } else if (getDB()) {
      DBInterface& db(*getDB());
      m_obj = DBObjectLoader::load(db, m_table, m_rcconfig, m_showall);
      db.close();
      m_obj.print(m_showall);
    } else if (m_provider_host.size() > 0 && m_provider_port > 0) {
      TCPSocket socket(m_provider_host, m_provider_port);
      try {
        socket.connect();
        TCPSocketWriter writer(socket);
        writer.writeInt(1);
        writer.writeString(m_table + "/" + m_rcconfig);
        TCPSocketReader reader(socket);
        m_obj.readObject(reader);
        m_obj.print(m_showall);
      } catch (const IOException& e) {
        socket.close();
        throw (IOException("Socket connection error : %s ", e.what()));
      }
      socket.close();
    }
  }
  add(new RCConfigHandler(*this, "rcconfig", m_obj.getName()));
  add(m_obj);
  try {
    initialize(m_obj);
  } catch (const RCHandlerException& e) {
    LogFile::fatal("Failed to initialize. %s. terminating process", e.what());
    term();
    exit(1);
  }
}

bool RCCallback::perform(NSMCommunicator& com) throw()
{
  NSMMessage msg(com.getMessage());
  const RCCommand cmd = msg.getRequestName();
  RCState state(getNode().getState());
  /*
  if (cmd == NSMCommand::VSET &&
      (state != RCState::OFF_S && state != RCState::UNKNOWN &&
       state != RCState::ERROR_ES && state != RCState::RECOVERING_RS &&
       state != RCState::ABORTING_RS && state != RCState::NOTREADY_S &&
       state != RCState::READY_S)) {
    return false;
  }
  */
  if (NSMCallback::perform(com)) return true;
  LogFile::info("%s:%d", __FILE__, __LINE__);
  if (cmd.isAvailable(state) ==  NSMCommand::DISABLED) {
    return false;
  }
  LogFile::info("%s:%d", __FILE__, __LINE__);
  LogFile::debug("%s >> %s (state = %s)", msg.getNodeName(),
                 cmd.getLabel(), state.getLabel());
  addNode(NSMNode(msg.getNodeName()));
  try {
    set("rcrequest", msg.getRequestName());
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
  LogFile::info("%s:%d", __FILE__, __LINE__);
  RCState tstate(cmd.nextTState());
  try {
    if (tstate != Enum::UNKNOWN) {
      LogFile::info("%s:%d", __FILE__, __LINE__);
      setState(tstate);
      if (cmd == RCCommand::CONFIGURE) {
        const NSMMessage& msg(com.getMessage());
        try {
          abort();
          dbload(msg.getLength(), msg.getData());
        } catch (const IOException& e) {
          throw (RCHandlerException(e.what()));
        }
        //set("rcconfig", m_obj.getName());
        configure(m_obj);
        setState(state);
        reply(NSMMessage(NSMCommand::OK, state.getLabel()));
      } else if (cmd == RCCommand::BOOT) {
        LogFile::info("%s:%d", __FILE__, __LINE__);
        get(m_obj);
        boot(m_obj);
      } else if (cmd == RCCommand::LOAD) {
        get(m_obj);
        load(m_obj);
      } else if (cmd == RCCommand::START) {
        m_expno = (msg.getNParams() > 0) ? msg.getParam(0) : 0;
        m_runno = (msg.getNParams() > 1) ? msg.getParam(1) : 0;
        start(m_expno, m_runno);
        if (m_expno > 0 && m_runno > 0) {
          dbrecord(m_obj, m_expno, m_runno, true);
        }
      } else if (cmd == RCCommand::STOP) {
        stop();
        if (m_expno > 0 && m_runno > 0) {
          dbrecord(m_obj, m_expno, m_runno, false);
          m_expno = 0;
          m_runno = 0;
        }
      } else if (cmd == RCCommand::RESUME) {
        if (!resume(msg.getParam(0))) {
          setState(RCState::NOTREADY_S);
          return true;
        }
      } else if (cmd == RCCommand::PAUSE) {
        if (!pause()) {
          setState(RCState::NOTREADY_S);
          return true;
        }
      }
      try {
        if (cmd == RCCommand::ABORT) {
          abort();
          if (m_expno > 0 && m_runno > 0) {
            dbrecord(m_obj, m_expno, m_runno, false);
            m_expno = 0;
            m_runno = 0;
          }
        } else if (cmd == RCCommand::RECOVER) {
          recover(m_obj);
        }
      } catch (const RCHandlerException& e) {
        LogFile::fatal("Failed to recover/abort : %s", e.what());
        reply(NSMMessage(NSMCommand::FATAL, e.what()));
      }
    }
    RCState state = cmd.nextState();
    if (state != Enum::UNKNOWN && m_auto) {
      setState(state);
    }
  } catch (const RCHandlerFatalException& e) {
    LogFile::fatal(e.what());
    reply(NSMMessage(NSMCommand::FATAL, e.what()));
  } catch (const RCHandlerException& e) {
    LogFile::error(e.what());
    setState(RCState::ERROR_ES);
    reply(NSMMessage(NSMCommand::ERROR, e.what()));
  } catch (const std::exception& e) {
    LogFile::fatal("Unknown exception: %s. terminating process", e.what());
  }
  return true;
}

void RCCallback::timeout(NSMCommunicator& /*com*/) throw()
{
  try {
    monitor();
  } catch (const RCHandlerFatalException& e) {
    LogFile::fatal(e.what());
    setState(RCState::ERROR_ES);
    reply(NSMMessage(NSMCommand::FATAL, e.what()));
  } catch (const RCHandlerException& e) {
    LogFile::error(e.what());
    setState(RCState::ERROR_ES);
    reply(NSMMessage(NSMCommand::ERROR, e.what()));
  } catch (const std::exception& e) {
    LogFile::fatal("Unknown exception: %s. terminating process", e.what());
  }
}

void RCCallback::setState(const RCState& state) throw()
{
  if (getNode().getState() != state) {
    LogFile::debug("state transit : %s >> %s",
                   getNode().getState().getLabel(), state.getLabel());
    getNode().setState(state);
    try {
      set("rcstate", state.getLabel());
    } catch (const std::exception& e) {
      LogFile::error(e.what());
    }
  }
  reply(NSMMessage(NSMCommand::OK, state.getLabel()));
}

void RCCallback::dbload(int length, const char* data)
throw(IOException)
{
  if (length > 0 && strlen(data) > 0) {
    remove(m_obj);
    StringList s = StringUtil::split(data, '/');
    if (s.size() == 0) {
      throw (DBHandlerException("Bad config name was selected : %s", data));
    }
    const std::string table = (s.size() > 1) ? s[0] : m_table;
    std::string config = (s.size() > 1) ? s[1] : s[0];
    if (table.size() == 0) {
      throw (DBHandlerException("Empty DB table name"));
    }
    if (config.size() == 0) {
      throw (DBHandlerException("Empty config name"));
    }
    if (!StringUtil::find(config, "@")) {
      config = getNode().getName() + "@" + config;
    }
    m_table = table;
    m_rcconfig = config;
    set("dbtable", m_table);
  }
  if (m_table.size() > 0 && m_rcconfig.size() > 0) {
    if (getDB()) {
      DBInterface& db(*getDB());
      try {
        m_obj = DBObjectLoader::load(db, m_table, m_rcconfig, m_showall);
        db.close();
        m_obj.print(m_showall);
      } catch (const DBHandlerException& e) {
        db.close();
        throw (e);
      }
    } else if (m_provider_host.size() > 0 && m_provider_port > 0) {
      TCPSocket socket(m_provider_host, m_provider_port);
      try {
        socket.connect();
        TCPSocketWriter writer(socket);
        writer.writeInt(1);
        writer.writeString(m_table + "/" + m_rcconfig);
        TCPSocketReader reader(socket);
        m_obj.readObject(reader);
        m_obj.print(m_showall);
      } catch (const IOException& e) {
        socket.close();
        throw (IOException("Socket connection error : %s ", e.what()));
      }
      socket.close();
    }
  } else {
    LogFile::warning("No DB objects was loaded");
  }
  add(m_obj);
}

void RCCallback::dbrecord(DBObject obj, int expno, int runno, bool isstart)
throw(IOException)
{
  if (m_table.size() > 0 && m_rcconfig.size() > 0) {
    //std::string confname = StringUtil::form("%04d:%06d:%s=", expno, runno, (isstart ? "s" : "e")) + m_rcconfig;
    std::string confname = getNode().getName() + StringUtil::form("@%04d:%06d:%s", expno, runno, (isstart ? "s" : "e"));
    obj.setName(confname);
    std::string table = m_table + "_log";
    if (getDB()) {
      DBInterface& db(*getDB());
      try {
        DBObjectLoader::createDB(db, table, obj);
        db.close();
      } catch (const DBHandlerException& e) {
        db.close();
        //throw (e);
      }
    } else if (m_provider_host.size() > 0 && m_provider_port > 0) {
      TCPSocket socket(m_provider_host, m_provider_port);
      try {
        socket.connect();
        TCPSocketWriter writer(socket);
        writer.writeInt(2);
        writer.writeString(table);
        obj.writeObject(writer);
      } catch (const IOException& e) {
        socket.close();
        throw (IOException("Socket connection error : %s ", e.what()));
      }
      socket.close();
    }
  } else {
    LogFile::warning("DB objects was not recoreded");
  }
}

