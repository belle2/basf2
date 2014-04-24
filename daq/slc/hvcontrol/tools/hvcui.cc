#include "daq/slc/hvcontrol/HVCommand.h"
#include "daq/slc/hvcontrol/HVState.h"
#include "daq/slc/hvcontrol/hv_status.h"

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

const char* getStateCode(const HVState& state)
{
  if (state == HVState::PEAK_S) {
    return "\x1b[42m\x1b[37m";
  } else if (state == HVState::OFF_S) {
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

void print(ConfigObject& obj, bool edited)
{
  std::stringstream ss, ss1;
  ss << StringUtil::form(" %5s | %4s | %4s | %6s | %10s | %10s | %10s | "
                         "%10s | %33s | %33s ", "crate",
                         "slot", "channel", "turnon",
                         "rampup_speed", "rampdown_speed",
                         "voltage_limit", "current_limit",
                         "voltage_demand", "reserved") << std::endl;
  int len = ss.str().size() - 1;
  for (int i = 0; i < len; i++) {
    ss1 << "-";
  }
  ss1 << std::endl;
  ss << ss1.str();
  ConfigObjectList& obj_v(obj.getObjects("channel"));
  for (size_t i = 0; i < obj_v.size(); i++) {
    ConfigObject& cobj(obj_v[i]);
    ss << StringUtil::form(" %5d | %4d | %7d | %6s | %12s | %14s | %13s | "
                           "%13s | %6s | %6s | %6s | %6s | %6s | "
                           "%6s | %6s | %6s ", cobj.getInt("crate"),
                           cobj.getInt("slot"), cobj.getInt("channel"),
                           (cobj.getBool("turnon") ? "true" : "false"),
                           StringUtil::form("%4.1f", cobj.getFloat("rampup_speed")).c_str(),
                           StringUtil::form("%4.1f", cobj.getFloat("rampdown_speed")).c_str(),
                           StringUtil::form("%4.1f", cobj.getFloat("voltage_limit")).c_str(),
                           StringUtil::form("%4.1f", cobj.getFloat("current_limit")).c_str(),
                           StringUtil::form("%4.1f", cobj.getFloat("voltage_demand_0")).c_str(),
                           StringUtil::form("%4.1f", cobj.getFloat("voltage_demand_1")).c_str(),
                           StringUtil::form("%4.1f", cobj.getFloat("voltage_demand_2")).c_str(),
                           StringUtil::form("%4.1f", cobj.getFloat("voltage_demand_3")).c_str(),
                           StringUtil::form("%4.1f", cobj.getFloat("reserved_0")).c_str(),
                           StringUtil::form("%4.1f", cobj.getFloat("reserved_1")).c_str(),
                           StringUtil::form("%4.1f", cobj.getFloat("reserved_2")).c_str(),
                           StringUtil::form("%4.1f", cobj.getFloat("reserved_3")).c_str()) << std::endl;
  }
  std::cout << "modename = " << obj.getName()
            << (edited ? "(edited)" : "") << std::endl
            << ss1.str() << ss.str() << ss1.str() << std::endl;
}

void edit(ConfigObject& obj, const std::string& str, bool& edited)
{
  std::vector<std::string> sarg_v = StringUtil::split(str, ' ');
  if (sarg_v.size() >= 5) {
    std::string name = sarg_v[0];
    if (name == "crate" || name == "slot" || name == "channel") return;
    int crate = (isdigit(sarg_v[1].at(0))) ? atoi(sarg_v[1].c_str()) : -1;
    int slot = (isdigit(sarg_v[2].at(0))) ? atoi(sarg_v[2].c_str()) : -1;
    int channel = (isdigit(sarg_v[3].at(0))) ? atoi(sarg_v[3].c_str()) : -1;
    std::string value = sarg_v[4];
    ConfigObjectList& obj_v(obj.getObjects("channel"));
    for (size_t i = 0; i < obj_v.size(); i++) {
      ConfigObject& cobj(obj_v[i]);
      if ((crate == -1 || crate == cobj.getInt("crate")) &&
          (slot == -1 || slot == cobj.getInt("slot")) &&
          (channel == -1 || channel == cobj.getInt("channel"))) {
        if (cobj.hasValue(name)) {
          cobj.setValueText(name, value);
          std::cout << "Set " << name << "[" << cobj.getInt("crate") << "]["
                    << cobj.getInt("slot") << "][" << cobj.getInt("channel") << "] = "
                    << value << std::endl;
          edited = true;
        } else {
          std::cout << "Unknown parameter : " << name << std::endl;
          return;
        }
      }
    }
  }
}

void nsmget(const std::string& hvnodename, hv_status* status)
{
  HVState s(status->state);
  std::stringstream ss1, ss;
  std::string l = StringUtil::form(" index | %13s | %7s | %7s", "state", "voltage", "current");
  int len = l.size();
  for (int i = 0; i < len; i++) {
    ss1 << "-";
  }
  ss1 << std::endl;
  ss << StringUtil::form("%s : %s %s \x1b[49m\x1b[39m", hvnodename.c_str(),
                         getStateCode(s), s.getLabel()) << std::endl
     << ss1.str() << l << std::endl << ss1.str();
  for (int i = 0; i < 9; i++) {
    hv_status::channel_status& ch_status(status->channel[i]);
    s = HVState(ch_status.state);
    ss << StringUtil::form(" %5d | %s%12s \x1b[49m\x1b[39m | %7s | %7s", i,
                           getStateCode(s), s.getLabel(),
                           StringUtil::form("%4.1f", ch_status.voltage_mon).c_str(),
                           StringUtil::form("%4.1f", ch_status.current_mon).c_str()) << std::endl;
  }
  ss << ss1.str();
  std::cout << "Updated at " << Date().toString() << std::endl
            << ss.str();
}

void dbget(const std::string& configname, const std::string& hvnodename,
           DBInterface* db, ConfigObject& obj)
{
  ConfigObjectTable table(db);
  db->connect();
  obj = table.get(configname, hvnodename);
  print(obj, false);
  db->close();
}

class NSMListener {

public:
  NSMListener(NSMCommunicator* comm, NSMNode& node,
              NSMNode& hvnode, hv_status* status)
    : m_comm(comm), m_node(node), m_hvnode(hvnode), m_status(status) {}
  virtual ~NSMListener() throw() {}

public:
  void run() {
    while (true) {
      if (m_comm->wait(10)) {
        NSMMessage& msg(m_comm->getMessage());
        HVCommand cmd(msg.getRequestName());
        if (cmd == NSMCommand::OK) {
          HVState s(msg.getData());
          fputs("\033[2J\033[0;0H", stdout);
          rewind(stdout);
          ftruncate(1, 0);
          std::cout << msg.getRequestName() << " : state = "
                    << getStateCode(s) << " " << s.getLabel()
                    << " \x1b[49m\x1b[39m" << std::endl;
          nsmget(m_hvnode.getName(), m_status);
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
  NSMNode& m_hvnode;
  hv_status* m_status;

};

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("Usage : ./hvcui nodename hvnode");
    return 1;
  }
  ConfigFile config("slowcontrol", "hv");
  std::string nodename = argv[1];
  std::string hvnodename = argv[2];
  NSMNode node(nodename);
  NSMNode hvnode(hvnodename);//config.get("hv.nsm.name"));
  NSMCommunicator* comm = new NSMCommunicator();
  comm->init(node, config.get("nsm.local.host"), config.getInt("nsm.local.port"));
  NSMData data(hvnodename + "_STATUS", "hv_status", 1);
  hv_status* status = (hv_status*)data.open(comm);
  PThread(new NSMListener(comm, node, hvnode, status));
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  db->connect();
  ConfigInfo cinfo = ConfigInfoTable(db).get(status->configid);
  ConfigObjectTable table(db);
  ConfigObject obj = table.get(cinfo.getName(), hvnodename);
  db->close();

  HIST_ENTRY* history = NULL;
  int nhistory = 0;
  char* prompt = (char*)malloc(nodename.size() + 3);
  strcpy(prompt, (nodename + "> ").c_str());
  char* line = NULL;
  bool edited = false;
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
      print(obj, edited);
    } else if (command == "EDIT") {
      std::cout << command << std::endl;
      edit(obj, str, edited);
    }
    HVCommand cmd = command;
    if (cmd == Enum::UNKNOWN) continue;
    std::cout << cmd.getLabel() << std::endl;
    if (cmd == NSMCommand::NSMGET) {
      nsmget(hvnodename, status);
    } else if (cmd == NSMCommand::DBGET) {
      std::vector<std::string> sarg_v = StringUtil::split(str, ' ');
      if (sarg_v.size() >= 1) {
        edited = false;
        dbget(sarg_v[0], hvnodename, db, obj);
      } else {
        ConfigObject obj_tmp;
        db->connect();
        ConfigInfoTable table(db);
        ConfigInfoList info_v = table.getList(hvnodename);
        db->close();
        for (size_t i = 0; i < info_v.size(); i++) {
          dbget(info_v[i].getName(), hvnodename, db, obj_tmp);
        }
      }
    } else if (cmd == NSMCommand::DBSET) {
      std::vector<std::string> sarg_v = StringUtil::split(str, ' ');
      if (sarg_v.size() >= 1) {
        db->connect();
        obj.setName(sarg_v[0]);
        table.add(obj, true);
        edited = false;
        db->close();
      }
    } else {
      try {
        if (cmd == HVCommand::CONFIGURE) {
          std::vector<std::string> sarg_v = StringUtil::split(str, ' ');
          if (sarg_v.size() >= 1) {
            comm->sendRequest(NSMMessage(hvnode, cmd, sarg_v[0]));
          } else {
            comm->sendRequest(NSMMessage(hvnode, cmd, obj.getName()));
          }
        } else {
          comm->sendRequest(NSMMessage(hvnode, cmd));
        }
      } catch (const NSMHandlerException& e) {

      }
    }
  }

  return 0;
}

