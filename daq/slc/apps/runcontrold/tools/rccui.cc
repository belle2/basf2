#include "daq/slc/runcontrol/RCCommand.h"
#include "daq/slc/runcontrol/RCState.h"

#include "daq/slc/apps/runcontrold/rc_status.h"

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/database/ConfigObjectTable.h>
#include <daq/slc/database/ConfigInfoTable.h>
#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <sstream>

extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
}

using namespace Belle2;

struct runsetting {
  std::string configname;
  std::string operators;
  std::string comment;
  bool stored;
} setting = {"", "", "", false};

const char* getStateCode(const RCState& state)
{
  if (state == RCState::RUNNING_S) {
    return "\x1b[42m\x1b[37m";
  } else if (state == RCState::OFF_S) {
    return "\x1b[47m\x1b[39m";
  } else if (state.isStable()) {
    return "\x1b[44m\x1b[37m";
  } else if (state.isTransition()) {
    return "\x1b[45m\x1b[37m";
  } else if (state.isError()) {
    return "\x1b[41m\x1b[37m";
  } else if (state == Enum::UNKNOWN) {
    return "\x1b[49m\x1b[31m";
  }
  return "\x1b[49m\x1b[39m";;
}

void nsmget(const std::string& rcnodename,
            ConfigObject& obj, rc_status* status)
{
  RCState s(status->state);
  std::stringstream ss1, ss;
  std::string l = StringUtil::form(" index | %9s | %13s ",
                                   "node", "state");
  int len = l.size();
  for (int i = 0; i < len; i++) {
    ss1 << "-";
  }
  ss1 << std::endl;
  ss << "Updated at  : " << Date().toString() << std::endl;
  ss << "Operators   : " << (setting.stored ? "" : "(") << setting.operators
     << (setting.stored ? "" : ")") << std::endl
     << "Comment     : " << (setting.stored ? "" : "(") << setting.comment
     << (setting.stored ? "" : ")") << std::endl
     << "RunType     : " << setting.configname << std::endl
     << "Started at  : " << ((status->stime > 0) ? Date(status->stime).toString() : "----")
     << std::endl
     << StringUtil::form("Mastor(%s) : %s %s \x1b[49m\x1b[39m", rcnodename.c_str(),
                         getStateCode(s), s.getLabel()) << std::endl
     << StringUtil::form("Run#        : %04d.%04d.%03d", status->expno,
                         status->runno, status->subno) << std::endl
     << ss1.str() << l << std::endl << ss1.str();
  ConfigObjectList& obj_v(obj.getObjects("node"));
  for (size_t i = 0; i < status->nnodes; i++) {
    rc_status::node_status& nstatus(status->node[i]);
    s = RCState(nstatus.state);
    ss << StringUtil::form(" %5d | %9s | %s%12s \x1b[49m\x1b[39m ", i,
                           obj_v[i].getObject("runtype").getNode().c_str(),
                           getStateCode(s), s.getLabel()) << std::endl;
  }
  ss << ss1.str();
  std::cout << ss.str();
}

void dbget(const std::string& configname, const std::string& rcnodename,
           DBInterface* db, ConfigObject& obj)
{
  ConfigObjectTable table(db);
  db->connect();
  obj = table.get(configname, rcnodename);
  obj.print();
  //print(obj, false);
  db->close();
}

class NSMListener {

public:
  NSMListener(NSMCommunicator* comm, NSMNode& node,
              ConfigObject& obj, NSMNode& rcnode,
              rc_status* status)
    : m_comm(comm), m_node(node), m_obj(obj),
      m_rcnode(rcnode), m_status(status) {}
  virtual ~NSMListener() throw() {}

public:
  void run() {
    while (true) {
      if (m_comm->wait(10)) {
        NSMMessage& msg(m_comm->getMessage());
        RCCommand cmd(msg.getRequestName());
        if (cmd == NSMCommand::OK) {
          RCState s(msg.getData());
          fputs("\033[2J\033[0;0H", stdout);
          rewind(stdout);
          ftruncate(1, 0);
          std::cout << msg.getRequestName() << " : state = "
                    << getStateCode(s) << " " << s.getLabel()
                    << " \x1b[49m\x1b[39m" << std::endl;
          nsmget(m_rcnode.getName(), m_obj, m_status);
          std::cout << m_node.getName() << "> ";
          std::cout.flush();
        } else if (cmd == NSMCommand::ERROR) {
          std::cout << " : message = " << msg.getData() << std::endl;
        }
      }
    }
  }

private:
  NSMCommunicator* m_comm;
  NSMNode& m_node;
  ConfigObject& m_obj;
  NSMNode& m_rcnode;
  rc_status* m_status;

};

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("Usage : ./rccui nodename rcnode");
    return 1;
  }
  ConfigFile config("slowcontrol", "rc");
  std::string nodename = argv[1];
  std::string rcnodename = argv[2];
  NSMNode node(nodename);
  NSMNode rcnode(rcnodename);//config.get("rc.nsm.name"));
  NSMCommunicator* comm = new NSMCommunicator();
  comm->init(node, config.get("nsm.local.host"), config.getInt("nsm.local.port"));
  NSMData data(rcnodename + "_STATUS", "rc_status", 1);
  rc_status* status = (rc_status*)data.open(comm);
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  db->connect();
  ConfigInfo cinfo = ConfigInfoTable(db).get(status->configid);
  ConfigObjectTable table(db);
  setting.configname = cinfo.getName();
  ConfigObject obj = table.get(setting.configname, rcnodename);
  db->close();
  HIST_ENTRY* history = NULL;
  int nhistory = 0;
  char* prompt = (char*)malloc(nodename.size() + 3);
  strcpy(prompt, (nodename + "> ").c_str());
  char* line = NULL;
  PThread(new NSMListener(comm, node, obj, rcnode, status));
  comm->sendRequest(NSMMessage(rcnode, RCCommand::STATECHECK));
  sleep(3);
  while (true) {
    line = readline(prompt);
    std::vector<std::string> str_v = StringUtil::split(line, ' ', 2);
    add_history(line);
    nhistory++;
    if (nhistory >= 20) {
      history = remove_history(0);
      free(history);
    }
    fputs("\033[2J\033[0;0H", stdout);
    rewind(stdout);
    ftruncate(1, 0);
    std::string command = StringUtil::toupper(str_v[0]);
    if (command == "QUIT") return 0;
    std::string str = (strlen(line) > command.size()) ? (line + command.size() + 1) : "";
    free(line);
    if (command == "PRINT") {
      std::cout << command << std::endl;
      ConfigObjectList& obj_v(obj.getObjects("node"));
      for (size_t i = 0; i < obj_v.size(); i++) {
        DBObject& cobj(obj_v[i].getObject("runtype"));
        std::string cnodename = cobj.getNode();
        if (str.size() == 0 || str == cnodename) {
          std::cout << cnodename << " (" << cobj.getTable() << ":" << cobj.getRevision() << ")" << std::endl;
          std::cout << "-------------------------" << std::endl;
          cobj.print();
          std::cout << "-------------------------" << std::endl;
          std::cout << std::endl;
        }
      }
      continue;
      //print(obj, edited);
    } else if (command == "EDIT") {
      std::vector<std::string> sarg_v = StringUtil::split(str, ' ', 2);
      std::string opt = StringUtil::toupper(sarg_v[0]);
      if (str.size() > opt.size()) str.erase(0, opt.size() + 1);
      else str = "";
      if (opt == "OPERATORS") {
        setting.operators = str;
        setting.stored = false;
      } else if (opt == "COMMENT") {
        setting.comment = str;
        setting.stored = false;
      } else {
        std::cout << "Usage: edit [operators / comment] arguments." << std::endl;
        continue;
      }
      std::cout << command << " " << opt << " " << str << std::endl;
      continue;
      //edit(obj, str, edited);
    }
    RCCommand cmd = command;
    if (cmd == Enum::UNKNOWN) {
      std::cout << "Unknown command : " << command << std::endl;
      continue;
    }
    if (cmd == NSMCommand::NSMGET) {
      nsmget(rcnodename, obj, status);
    } else if (cmd == NSMCommand::DBGET) {
      std::vector<std::string> sarg_v = StringUtil::split(str, ' ');
      if (sarg_v.size() >= 1) {
        //edited = false;
        setting.configname = sarg_v[0];
        dbget(setting.configname, rcnodename, db, obj);
      } else {
        ConfigObject obj_tmp;
        db->connect();
        ConfigInfoTable table(db);
        ConfigInfoList info_v = table.getList(rcnodename);
        db->close();
        for (size_t i = 0; i < info_v.size(); i++) {
          dbget(info_v[i].getName(), rcnodename, db, obj_tmp);
        }
      }
    } else if (cmd == NSMCommand::DBSET) {
      std::vector<std::string> sarg_v = StringUtil::split(str, ' ');
      if (sarg_v.size() >= 1) {
        db->connect();
        setting.configname = sarg_v[0];
        obj.setName(setting.configname);
        table.add(obj, true);
        //edited = false;
        db->close();
      }
    } else {
      try {
        if (cmd == RCCommand::LOAD) {
          std::vector<std::string> sarg_v = StringUtil::split(str, ' ');
          if (sarg_v.size() >= 1) {
            comm->sendRequest(NSMMessage(rcnode, cmd, NSMCommand::DBGET.getId(), sarg_v[0]));
          } else {
            comm->sendRequest(NSMMessage(rcnode, cmd, NSMCommand::DBGET.getId(), obj.getName()));
          }
        } else if (cmd == RCCommand::START) {
          if (setting.operators.size() == 0) {
            std::cout << "operators are not set yet : edit operators <name>" << std::endl;
            continue;
          }
          if (setting.comment.size() == 0) {
            std::cout << "comment are not set yet : edit comment <comments>" << std::endl;
          }
          comm->sendRequest(NSMMessage(rcnode, cmd, setting.operators + "\n" + setting.comment));
          setting.stored = true;
        } else {
          comm->sendRequest(NSMMessage(rcnode, cmd));
        }
      } catch (const NSMHandlerException& e) {

      }
    }
  }

  return 0;
}

