#include "HVControlCallback.h"
#include "ArichHVMessage.h"
#include "HVCrateInfo.h"
#include "SocketAcceptor.h"
#include "HVCommand.h"

#include <database/MySQLInterface.h>

#include <nsm/NSMNodeDaemon.h>
#include <nsm/NSMCommunicator.h>

#include <system/TCPSocketWriter.h>

#include <base/ConfigFile.h>
#include <base/Debugger.h>
#include <base/StringUtil.h>

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <unistd.h>

extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
}

using namespace Belle2;

void show(HVChannelInfo*, std::string, std::string);
void set(HVChannelInfo*, std::string, std::string);
void get(HVChannelInfo*, std::string, std::string);

void save(HVCrateInfo*, std::string);
void recall(HVCrateInfo*, std::string, int);

HVCrateInfo* crate = new HVCrateInfo(0, 1, 4);

class ParamProvider {
public:
  ParamProvider(TCPSocket& socket)
    : _socket(socket) {}
public:
  void run() {
    try {
      TCPSocketWriter writer(_socket);
      writer.writeObject(*crate);
      sleep(10);
      _socket.close();
    } catch (const IOException& e) {
      Belle2::debug("Socket error: %s", e.what());
    }
  }
private:
  TCPSocket _socket;
};

int main(int argc, char** argv)
{
  if (argc < 1) {
    Belle2::debug("Usage : ./hvmaster");
    return 1;
  }

  ConfigFile config("slc_config");
  PThread(new SocketAcceptor<ParamProvider>(config.get("HV_GLOBAL_HOST"),
                                            config.getInt("HV_GLOBAL_PORT")));
  const std::string name = config.get("HV_NSM_NAME");
  NSMNode* node = new NSMNode(name);
  HVControlCallback* callback = new HVControlCallback(node);
  PThread(new NSMNodeDaemon(callback));
  sleep(2);

  node = new NSMNode();
  HIST_ENTRY* history = NULL;
  int nhistory = 0;
  char* prompt = (char*)malloc(name.size() + 2);
  strcpy(prompt, name.c_str());
  strcat(prompt, ">");
  char* line = NULL;
  while (true) {
    line = readline(prompt);
    std::vector<std::string> str_v = Belle2::split(line, ' ');
    add_history(line);
    nhistory++;
    if (nhistory >= 20) {
      history = remove_history(0);
      free(history);
    }
    free(line);
    str_v[0] = Belle2::tolower(str_v[0]);
    if (str_v[0] == "quit") {
      break;
    } else if (str_v[0] == "help") {
      std::cout << "usage:" << std::endl;
      std::cout << "show [<slot> <channel>]" << std::endl;
      std::cout << "set  [<slot> <channel>] <param_name> <value>" << std::endl;
      std::cout << "get  [<slot> <channel>] <param_name>" << std::endl;
      std::cout << "save [<slot> <channel>]" << std::endl;
      std::cout << "quit => quit program" << std::endl;
    } else if (str_v[0] == "load") {
      NSMCommunicator* comm = callback->getCommunicator();
      if (comm != NULL) {
        try {
          node->setName(str_v[1]);
          comm->sendRequest(node, HVCommand::LOAD);
        } catch (const IOException& e) {
          Belle2::debug("NSM error : %s ", e.what());
        }
      }
    } else if (str_v[0] == "switch") {
      if (str_v.size() < 3) continue;
      NSMCommunicator* comm = callback->getCommunicator();
      HVCommand command = (Belle2::tolower(str_v[1]) == "on") ?
                          HVCommand::SWITCHON : HVCommand::SWITCHOFF;
      if (comm != NULL) {
        try {
          node->setName(str_v[2]);
          comm->sendRequest(node, command);
        } catch (const IOException& e) {
          Belle2::debug("NSM error : %s ", e.what());
        }
      }
    } else if (str_v[0] == "save") {
      if (str_v.size() > 1)
        save(crate, str_v[1]);
    } else if (str_v[0] == "recall") {
      if (str_v.size() > 2)
        recall(crate, str_v[1], atoi(str_v[2].c_str()));
    } else {
      size_t slot = 0;
      size_t ch = 0;
      void (* func)(HVChannelInfo*, std::string, std::string);
      if (str_v[0] == "get") func = get;
      else if (str_v[0] == "set") func = set;
      else if (str_v[0] == "show") func = show;
      if (str_v.size() > 1) slot = atoi(str_v[1].c_str());
      if (str_v.size() > 2) ch = atoi(str_v[2].c_str());
      std::string pname = (str_v.size() > 3) ? str_v[3] : "";
      std::string value = (str_v.size() > 4) ? str_v[4] : "";
      if (slot == 0) {
        for (size_t ns = 0; ns < crate->getNSlot(); ns++) {
          if (ch == 0) {
            for (size_t nc = 0; nc < crate->getNChannel(); nc++) {
              func(crate->getChannel(ns, nc), pname, value);
            }
          } else {
            func(crate->getChannel(ns, ch - 1), pname, value);
          }
        }
      } else {
        if (ch == 0) {
          for (size_t nc = 0; nc < crate->getNChannel(); nc++) {
            func(crate->getChannel(slot - 1, nc), pname, value);
          }
        } else {
          func(crate->getChannel(slot - 1, ch - 1), pname, value);
        }
      }
    }
  }

  return 0;
}

void show(HVChannelInfo* info, std::string, std::string)
{
  info->print();
  std::cout << std::endl;
}

void save(HVCrateInfo* crate, std::string nodename)
{
  ConfigFile config;
  MySQLInterface db;
  db.init();
  db.connect(config.get("HV_DATABASE_HOST"), config.get("HV_DATABASE_NAME"),
             config.get("HV_DATABASE_USER"), config.get("HV_DATABASE_PASS"),
             config.getInt("HV_DATABASE_PORT"));
  try {
    db.execute(Belle2::form("create table hv_database (node text, %s);",
                            crate->getChannel(0, 0)->toSQLConfig().c_str()));
  } catch (const IOException& e) {}
  int confno = 1;
  try {
    db.execute("select confno from hv_database order by confno desc limit 1;");
    std::vector<DBRecord>& record_v(db.loadRecords());
    if (record_v.size() > 0)
      confno = atoi(record_v[0].getFieldValue("confno").c_str()) + 1;
  } catch (const IOException& e) {}
  for (size_t ns = 0; ns < crate->getNSlot(); ns++) {
    for (size_t nc = 0; nc < crate->getNChannel(); nc++) {
      HVChannelInfo* info = crate->getChannel(ns, nc);
      info->setConfigNumber(confno);
      db.execute(Belle2::form("insert into hv_database (node, %s) values ('%s', %s);",
                              info->toSQLNames().c_str(),
                              nodename.c_str(), info->toSQLValues().c_str()));
    }
  }
  std::cout << "Save with configuration number = " << confno << std::endl;
  db.close();
}

void recall(HVCrateInfo* crate, std::string nodename, int confno)
{
  ConfigFile config;
  MySQLInterface db;
  db.init();
  db.connect(config.get("HV_DATABASE_HOST"), config.get("HV_DATABASE_NAME"),
             config.get("HV_DATABASE_USER"), config.get("HV_DATABASE_PASS"),
             config.getInt("HV_DATABASE_PORT"));
  try {
    db.execute(Belle2::form("select * from hv_database where node = '%s' and confno = %d;",
                            nodename.c_str(), confno));
    std::vector<DBRecord>& record_v(db.loadRecords());
    for (size_t i = 0; i < record_v.size(); i++) {
      unsigned int slot = record_v[i].getFieldValueInt("slot");
      unsigned int ch = record_v[i].getFieldValueInt("channel");
      if (slot < crate->getNSlot() + 1 && ch < crate->getNChannel() + 1) {
        HVChannelInfo* info = crate->getChannel(slot - 1, ch - 1);
        info->setSwitchOn(record_v[i].getFieldValueInt("slot") == 1);
        info->setVoltageDemand(record_v[i].getFieldValueInt("voltage_demand"));
        info->setVoltageLimit(record_v[i].getFieldValueInt("voltage_limit"));
        info->setCurrentLimit(record_v[i].getFieldValueInt("current_limit"));
        info->setRampUpSpeed(record_v[i].getFieldValueInt("rampup_speed"));
        info->setRampDownSpeed(record_v[i].getFieldValueInt("rampdown_speed"));
      }
    }
  } catch (const IOException& e) {}
  db.close();
}

void set(HVChannelInfo* info, std::string label, std::string value)
{
  label = Belle2::toupper(label);
  if (label == "SWITCH") {
    info->setSwitchOn(Belle2::toupper(value) == "ON");
  } else if (label == "RAMPUP_SPEED") {
    info->setRampUpSpeed(strtoul(value.c_str(), 0, 0));
  } else if (label == "RAMPDOWN_SPEED") {
    info->setRampDownSpeed(strtoul(value.c_str(), 0, 0));
  } else if (label == "VOLTAGE_DEMAND") {
    info->setVoltageDemand(strtoul(value.c_str(), 0, 0));
  } else if (label == "VOLTAGE_LIMIT") {
    info->setVoltageLimit(strtoul(value.c_str(), 0, 0));
  } else if (label == "CURRENT_LIMIT") {
    info->setCurrentLimit(strtoul(value.c_str(), 0, 0));
  } else {
    std::cout << "Unknown parmater: " << label << std::endl;
  }
}

void get(HVChannelInfo* info, std::string label, std::string)
{
  label = Belle2::toupper(label);
  if (label == "SWITCH") {
    std::cout << info->isSwitchOn() << std::endl;
  } else if (label == "RAMPUP_SPEED") {
    std::cout << info->getRampUpSpeed() << std::endl;
  } else if (label == "RAMPDOWN_SPEED") {
    std::cout << info->getRampDownSpeed() << std::endl;
  } else if (label == "VOLTAGE_DEMAND") {
    std::cout << info->getVoltageDemand() << std::endl;
  } else if (label == "VOLTAGE_LIMIT") {
    std::cout << info->getVoltageLimit() << std::endl;
  } else if (label == "CURRENT_LIMIT") {
    std::cout << info->getCurrentLimit() << std::endl;
  } else {
    std::cout << "Unknown parmater: " << label << std::endl;
  }
}

