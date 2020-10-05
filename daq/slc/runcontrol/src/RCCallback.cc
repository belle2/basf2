#include "daq/slc/runcontrol/RCCallback.h"

#include <daq/slc/database/DBHandlerException.h>
#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/runcontrol/RCCommand.h>
#include <daq/slc/runcontrol/RCHandlerException.h>
#include <daq/slc/runcontrol/RCHandlerFatalException.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/TCPSocketWriter.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <sstream>

#include <cstdlib>

double tabort = 0;

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
      try {
        m_callback.abort();
        m_callback.dbload(val.size(), val.c_str());
      } catch (const IOException& e) {
        throw (RCHandlerException(e.what()));
      }
      DBObject& obj(m_callback.getDBObject());
      obj.getName();
      m_callback.configure(obj);
      m_callback.setState(state);
      return true;
    }
  private:
    RCCallback& m_callback;
  };

}

using namespace Belle2;

RCCallback::RCCallback(int timeout)
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
  reg(RCCommand::STATUS);
  reg(NSMCommand::FATAL);
  m_auto = true;
  m_db = NULL;
  m_showall = true;
  m_expno = m_runno = 0;
}

void RCCallback::init(NSMCommunicator&)
{
  LogFile::debug("init");
  dbload(0, 0);
  try {
    initialize(m_obj);
  } catch (const RCHandlerException& e) {
    LogFile::fatal("Failed to initialize. %s. terminating process (84)", e.what());
    term();
    exit(1);
  }
  LogFile::debug("init done");
  setState(RCState::NOTREADY_S);
  tabort = Date().get();
}

bool RCCallback::perform(NSMCommunicator& com)
{
  NSMMessage msg(com.getMessage());
  const RCCommand cmd = msg.getRequestName();
  const RCState state_org(getNode().getState());
  RCState state(getNode().getState());
  if (NSMCallback::perform(com)) return true;
  if (cmd.isAvailable(state) ==  NSMCommand::DISABLED) {
    return false;
  }
  addNode(NSMNode(msg.getNodeName()));
  try {
    set("rcrequest", msg.getRequestName());
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
  RCState tstate(cmd.nextTState());
  try {
    if (tstate != Enum::UNKNOWN) {
      log(LogFile::DEBUG, "RC request %s from %s", msg.getRequestName(), msg.getNodeName());
      setState(tstate);
      std::string nodename = getNode().getName();
      bool ismaster = nodename == "RUNCONTROL" ||
                      (StringUtil::find(nodename, "RC_") && !StringUtil::find(nodename, "HLT"));
      if (cmd == RCCommand::CONFIGURE) {
        m_runcontrol.setName(msg.getNodeName());
        configure_raw(msg.getLength(), msg.getData());
      } else if (cmd == RCCommand::BOOT) {
        m_runcontrol.setName(msg.getNodeName());
        get(m_obj);
        std::string opt = msg.getLength() > 0 ? msg.getData() : "";
        boot(opt, m_obj);
      } else if (cmd == RCCommand::LOAD) {
        m_expno = 0;
        m_runno = 0;
        m_runcontrol.setName(msg.getNodeName());
        std::string runtype = (msg.getLength() > 0 ? msg.getData() : "");
        if (runtype.size() == 0) {
          get("runtype", runtype);
        } else if (runtype.size() > 0) {
          set("runtype", runtype);
        }
        get(m_obj);
        load(m_obj, runtype);
      } else if (cmd == RCCommand::START) {
        m_runcontrol.setName(msg.getNodeName());
        m_expno = (msg.getNParams() > 0) ? msg.getParam(0) : 0;
        m_runno = (msg.getNParams() > 1) ? msg.getParam(1) : 0;
        if (ismaster) {
          log(LogFile::NOTICE, "Run start by %s (exp=%05d, run=%06d)",
              msg.getNodeName(), m_runno, m_expno);
        }
        start(m_expno, m_runno);
      } else if (cmd == RCCommand::STOP) {
        if (ismaster) {
          log(LogFile::NOTICE, "Run stop by %s (exp=%05d, run=%06d)",
              msg.getNodeName(), m_runno, m_expno);
        }
        stop();
      } else if (cmd == RCCommand::RESUME) {
        if (ismaster) {
          log(LogFile::NOTICE, "Run resume by %s (exp=%05d, run=%06d)",
              msg.getNodeName(), m_runno, m_expno);
        }
        if (!resume(msg.getParam(0))) {
          setState(RCState::NOTREADY_S);
          return true;
        }
      } else if (cmd == RCCommand::PAUSE) {
        if (ismaster) {
          log(LogFile::NOTICE, "Run pause by %s (exp=%05d, run=%06d)",
              msg.getNodeName(), m_runno, m_expno);
        }
        if (!pause()) {
          setState(RCState::NOTREADY_S);
          return true;
        }
      }
      try {
        if (cmd == RCCommand::ABORT) {
          double t = Date().get();
          if (t - tabort > 3) {
            if (ismaster) {
              log(LogFile::NOTICE, "Run abort by %s (exp=%05d, run=%06d)",
                  msg.getNodeName(), m_runno, m_expno);
            }
            m_runcontrol.setName(msg.getNodeName());
            abort();
          }
          tabort = t;
        } else if (cmd == RCCommand::RECOVER) {
          std::string runtype;
          get("runtype", runtype);
          recover(m_obj, runtype);
        }
      } catch (const RCHandlerException& e) {
        log(LogFile::FATAL, "Failed to recover/abort : %s", e.what());
      }
    }
    RCState state = cmd.nextState();
    if (getNode().getState() == tstate &&
        state != Enum::UNKNOWN && m_auto) {
      setState(state);
    }
    state = getNode().getState();
    if (state != Enum::UNKNOWN) {
      if ((cmd == RCCommand::START &&
           (state == RCState::RUNNING_S || state == RCState::STARTING_TS)) ||
          ((cmd == RCCommand::STOP || cmd == RCCommand::ABORT)
           && state_org == RCState::RUNNING_S))  {
        try {
          dump(cmd == RCCommand::START);
        } catch (const DBHandlerException& e) {
          LogFile::error(e.what());
        }
      }
    }
  } catch (const RCHandlerFatalException& e) {
    log(LogFile::FATAL, e.what());
  } catch (const RCHandlerException& e) {
    log(LogFile::ERROR, e.what());
    setState(RCState::ERROR_ES);
  } catch (const std::exception& e) {
    log(LogFile::FATAL, "Unknown exception: %s. terminating process (193)", e.what());
  }
  return true;
}

void RCCallback::dump(bool isstart)
{
  std::string runtype;
  get("runtype", runtype);
  std::string rcconfig = getNode().getName() + "@RC:"
                         + (isstart ? "start:" : "end:") + runtype
                         + StringUtil::form(":%05d:%04d", m_expno, m_runno);
  std::stringstream ss;
  ss << dbdump() << std::endl;
  ss << "config : " << rcconfig << std::endl;
  LogFile::debug(ss.str());
  ConfigFile file(ss);
  DBObject obj = DBObjectLoader::load(file);
  obj.print();
  std::string table = m_table + "record";
  if (getDB()) {
    DBInterface& db(*getDB());
    try {
      DBObjectLoader::createDB(db, table, obj);
    } catch (const IOException& e) {
      throw (DBHandlerException("Failed to connect to database error : %s ", e.what()));
    }
  } else if (m_provider_host.size() > 0 && m_provider_port > 0) {
    TCPSocket socket(m_provider_host, m_provider_port);
    try {
      socket.connect();
      TCPSocketWriter writer(socket);
      writer.writeInt(2);
      writer.writeString(table);
      writer.writeObject(obj);
    } catch (const IOException& e) {
      socket.close();
      throw (DBHandlerException("Failed to connect to dbprovider error : %s ", e.what()));
    }
    socket.close();
  }
}

void RCCallback::timeout(NSMCommunicator& /*com*/)
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
    LogFile::fatal("Unknown exception: %s. terminating process (249)", e.what());
  }
}

std::string RCCallback::dbdump()
{
  std::stringstream ss;
  StringList& hnames(getHandlerNames());
  NSMVHandlerList& handlers(getHandlers());
  for (StringList::iterator it = hnames.begin();
       it != hnames.end(); it++) {
    std::string hname = *it;
    NSMVHandler& handler(*handlers[hname]);
    std::string vname = StringUtil::replace(hname, "@", "");
    if (!handler.useGet()) {
      continue;
    }
    if (!handler.isDumped()) {
      continue;
    }
    if (vname.c_str()[0] == '.') {
      continue;
    }
    NSMVar var;
    handler.handleGet(var);
    switch (var.getType()) {
      case NSMVar::INT:
        ss << vname << " : int(" << var.getInt() << ")" << std::endl;
        break;
      case NSMVar::FLOAT:
        ss << vname << " : float(" << var.getFloat() << ")" << std::endl;
        break;
      case NSMVar::TEXT:
        ss << vname << " : \"" << var.getText() << "\"" << std::endl;
        break;
      default:
        break;
    }
  }
  ss << getDBObject().sprint(true) << std::endl;
  ss << "nodename : " << std::endl;
  return ss.str();
}

void RCCallback::setState(const RCState& state)
{
  RCState state_org = getNode().getState();
  if (state_org != state) {
    LogFile::debug("state transit : %s >> %s",
                   state_org.getLabel(), state.getLabel());
    try {
      getNode().setState(state);
      set("rcstate", state.getLabel());
    } catch (const std::exception& e) {
      LogFile::error(e.what());
    }
  }
}

DBObject RCCallback::dbload(const std::string& path)
{
  DBObject obj;
  std::string pathin, table, config;
  LogFile::debug(path);
  if (path.find("db://") != std::string::npos) {
    pathin = StringUtil::replace(path, "db://", "");
    StringList s = StringUtil::split(pathin, '/');
    if (s.size() > 1) {
      table = s[0];
      config = s[1];
    } else {
      table = m_table;
      config = s[0];
    }
  } else if (path.find("file://") != std::string::npos) {
    pathin = StringUtil::replace(path, "file:/", "");
    return obj;
  }
  if (table.size() > 0 && config.size() > 0) {
    if (getDB()) {
      DBInterface& db(*getDB());
      try {
        obj = DBObjectLoader::load(db, table, config, false);
        if (obj.getName().size() > 0) {
          config = obj.getName();
        }
        db.close();
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
        writer.writeString(table + "/" + config);
        TCPSocketReader reader(socket);
        obj.readObject(reader);
        socket.close();
        if (obj.getName().size() > 0) {
          config = obj.getName();
        }
      } catch (const IOException& e) {
        socket.close();
        throw (IOException("Socket connection error : %s ", e.what()));
      }
    }
  }
  return obj;
}

void RCCallback::configure_raw(int length, const char* data)
{
  try {
    dbload(length, data);
  } catch (const IOException& e) {
    throw (RCHandlerException(e.what()));
  }
  configure(m_obj);
}

void RCCallback::dbload(int /*length*/, const char* /*data*/)
{
  NSMNode& node(getNode());
  m_rcconfig = node.getName() + "@RC:" + m_rcconfig_org;
  LogFile::debug("Loading '%s'", m_rcconfig.c_str());
  if (m_file.size() > 0) {
    StringList files = StringUtil::split(m_file, ',');
    ConfigFile conf;
    for (size_t i = 0; i < files.size(); i++) {
      conf.read(files[i]);
    }
    m_obj = DBObjectLoader::load(conf);
    m_obj.print(m_showall);
  } else if (getDB()) {
    DBInterface& db(*getDB());
    std::string rcconfig = "";
    DBObject obj1, obj2;
    if (m_runtype_record.size()) {
      rcconfig = node.getName() + "@RC:start:" + m_runtype_record + ":";
      obj1 = DBObjectLoader::load(db, m_table + "record", rcconfig, m_showall);
    }
    obj2 = DBObjectLoader::load(db, m_table, m_rcconfig, m_showall);
    if (obj1.getDate() < obj2.getDate()) {
      m_obj = obj2;
    } else {
      m_obj = obj1;
      m_rcconfig = rcconfig;
    }
    db.close();
    m_obj.print(m_showall);
  } else if (m_provider_host.size() > 0 && m_provider_port > 0) {
    TCPSocket socket(m_provider_host, m_provider_port);
    try {
      DBObject obj1, obj2;
      std::string rcconfig = "";
      if (m_runtype_record.size() > 0) {
        rcconfig = node.getName() + "@RC:start:" + m_runtype_record + ":";
        socket.connect();
        TCPSocketWriter writer(socket);
        writer.writeInt(1);
        writer.writeString(m_table + "record/" + rcconfig);
        TCPSocketReader reader(socket);
        obj1.readObject(reader);
        socket.close();
      }
      socket.connect();
      TCPSocketWriter writer(socket);
      writer.writeInt(1);
      writer.writeString(m_table + "/" + m_rcconfig);
      TCPSocketReader reader(socket);
      obj2.readObject(reader);
      if (obj1.getDate() < obj2.getDate()) {
        m_obj = obj2;
      } else {
        m_obj = obj1;
        m_rcconfig = rcconfig;
      }
      m_obj.print(m_showall);
    } catch (const IOException& e) {
      socket.close();
      throw (IOException("Socket connection error : %s ", e.what()));
    }
    socket.close();
  }
  reset();
  add(new NSMVHandlerText("runtype", true, true, ""), false, true);
  add(new NSMVHandlerText("dbtable", true, false, m_table), false, true);
  add(new NSMVHandlerText("rcstate", true, false, RCState::NOTREADY_S.getLabel()), false, true);
  add(new NSMVHandlerText("rcrequest", true, false, ""), false, true);
  add(new RCConfigHandler(*this, "rcconfig", m_obj.getName()));
  addDB(m_obj);
}

