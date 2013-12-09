#include "daq/slc/apps/hvcontrold/HVControlMaster.h"

#include "daq/slc/apps/hvcontrold/HVCommand.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include "daq/slc/system/TCPSocket.h"
#include "daq/slc/system/TCPSocketReader.h"
#include "daq/slc/system/TCPSocketWriter.h"

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <cstdlib>

using namespace Belle2;

void HVControlMaster::add(const std::string& name, HVCrateInfo* crate)
{
  _mutex.lock();
  _crate_m.insert(HVCrateInfoMap::value_type(name, crate));
  _node_m.insert(NSMNodeMap::value_type(name, new NSMNode(name)));
  _mutex.unlock();
}

void HVControlMaster::accept(const TCPSocket& socket)
{
  _mutex.lock();
  try {
    TCPSocketReader reader(socket);
    std::vector<std::string> str_v = Belle2::split(reader.readString(), ':');
    const std::string nodename = str_v[0];
    if (_crate_m.find(nodename) != _crate_m.end()) {
      if (str_v[1] == "R") {
        TCPSocketWriter writer(socket);
        _crate_m[nodename]->writeInfo(writer);
      } else if (str_v[1] == "S") {
        _crate_m[nodename]->readStatus(reader);
      }
    }
  } catch (const IOException& e) {

  }
  _mutex.unlock();
}

void HVControlMaster::perform(const std::string& command, const std::string& str)
{
  _mutex.lock();
  if (command == "HELP") help(str);
  else if (command == "SAVE") save(str);
  else if (command == "RECALL") recall(str);
  else if (command == "GET") get(str);
  else if (command == "SET") set(str);
  else if (command == "SHOW") show(str);
  else if (command == "LIST") list(str);
  else if (command == "REMOVE") remove(str);
  else if (command == "LOAD") execute(str, HVCommand::LOAD);
  else if (command == "SWITCHON") execute(str, HVCommand::SWITCHON);
  else if (command == "SWITCHOFF") execute(str, HVCommand::SWITCHOFF);
  else Belle2::debug("Unknown command : %s", command.c_str());
  _mutex.unlock();
}

bool HVControlMaster::help(const std::string& str)
{
  if (str.size() == 0) {
    std::cout << "usage:" << std::endl
              << "help      [<command>]" << std::endl
              << "show      [<info/status>] [<nodename>] [<slot>] [<channel>]" << std::endl
              << "set       [<nodename>] [<slot>] [<channel>] <param_name> <value>" << std::endl
              << "get       [<nodename>] [<slot>] [<channel>] <param_name>" << std::endl
              << "save      [<nodename>]" << std::endl
              << "recall    <nodename> <config_no>" << std::endl
              << "remove    [<nodename> <config_no>]" << std::endl
              << "list      [<nodename>]" << std::endl
              << "load      [<nodename>]" << std::endl
              << "switchon  [<nodename>]" << std::endl
              << "switchoff [<nodename>]" << std::endl
              << "quit" << std::endl;
  } else if (str == "show") {
    std::cout << "show <nodename> [<slot> <channel>] : "
              << "Displays all parameters for slot = <slot> and channel = <channel> in <nodename>." << std::endl
              << "                                                 "
              << "Without <slot> and <channel>, Displays all parameters." << std::endl;
  } else if (str == "set") {
    std::cout << "set <nodename> [<slot> <channel>] <param_name> <value> : "
              << "Sets value of <param_name> as <value> for slot = "
              << "<slot> and channel = <channel> in <nodename>." << std::endl
              << "                                                         "
              << "Without <slot> and <channel>, Sets values of all channels as <values>." << std::endl;
  } else if (str == "get") {
    std::cout << "get <nodename> [<slot> <channel>] <param_name> : "
              << "Displays value of <param_name> for slot = <slot> and channel = <channel> in <nodename>." << std::endl
              << "                                                 "
              << "Without <slot> and <channel>, Displays values of all channels." << std::endl;
  } else if (str == "save") {
    std::cout << "save <nodename> : "
              << "Save current parameters of <nodename> as new entry to database." << std::endl;
  } else if (str == "recall") {
    std::cout << "list <nodename> [<config_no>] : "
              << "Load latest parameters of <nodename> from databsse." << std::endl
              << "                                "
              << "With load parameters with configuration # of <config_no>." << std::endl;
  } else if (str == "list") {
    std::cout << "list [<nodename>] : Send request to nodes to load parameters from master." << std::endl
              << "                    With argument <nodename>, send only to <nodename>." << std::endl;
  } else if (str == "load") {
    std::cout << "load <nodename> : Send request to <nodename> to load parameters from master." << std::endl
              << "                  With argument <nodename>, send only to <nodename>." << std::endl;
  } else if (str == "switchon") {
    std::cout << "switchon <nodename> : "
              << "Send request to <nodename> to switch on channels with switch_on == true." << std::endl
              << "                      With argument <nodename>, send only to <nodename>." << std::endl;
  } else if (str == "switchoff") {
    std::cout << "switchoff <nodename> : Send request to <nodename> to switch off all channels." << std::endl
              << "                       With argument <nodename>, send only to <nodename>." << std::endl;
  } else if (str == "quit") {
    std::cout << "quit : Quit program." << std::endl;
  } else {
    std::cout << "Unknown command: " << str << std::endl;
  }
  return true;
}

bool HVControlMaster::show(const std::string& s)
{
  std::vector<std::string> str_v = Belle2::split(s, ' ');
  std::string opt = "";
  std::string nodename;
  size_t slot = 0;
  size_t channel = 0;
  for (size_t i = 0; i < str_v.size(); i++) {
    std::string& str(str_v[i]);
    if (str.size() == 0) continue;
    else if (str == "INFO" || str == "STATUS") {
      opt = str;
    } else if (isdigit(str.at(0))) {
      if (slot == 0)
        slot = atoi(str.c_str());
      else if (channel == 0)
        channel = atoi(str.c_str());
    } else if (_crate_m.find(str) != _crate_m.end()) {
      nodename = str;
    } else {
      std::cout << "Unknown nodename : " << str << std::endl
                << "Requested : show opt <nodename> <slot> <channel>]" << std::endl;
      return false;
    }
  }
  if (opt != "INFO" && opt != "STATUS") opt = "INFO STATUS";
  HVCrateInfo* crate = _crate_m.begin()->second;
  std::vector<std::string> opt_v = Belle2::split(opt, ' ');
  for (size_t i = 0; i < opt_v.size(); i++) {
    if (opt_v[i] == "INFO") {
      std::cout << "HV channel info:" << std::endl;
      std::cout << "  node | " << crate->getChannelInfo(0, 0)->print_names() << std::endl;
    } else if (opt_v[i] == "STATUS") {
      std::cout << "HV channel status:" << std::endl;
      std::cout << "  node | " << crate->getChannelStatus(0, 0)->print_names() << std::endl;
    }
    if (nodename.size() > 0) {
      handle(show_local, _crate_m[nodename], slot, channel, nodename, opt_v[i]);
    } else {
      for (HVCrateInfoMap::iterator it = _crate_m.begin();
           it != _crate_m.end(); it++) {
        if (!handle(show_local, it->second, slot, channel, it->first, opt_v[i]))
          return false;
      }
    }
  }
  return true;
}

bool HVControlMaster::set(const std::string& s)
{
  std::vector<std::string> str_v = Belle2::split(s, ' ');
  size_t slot = 0;
  size_t channel = 0;
  std::string pname;
  std::string value;
  std::string nodename;
  for (size_t i = 0; i < str_v.size(); i++) {
    std::string& str(str_v[i]);
    if (str.size() == 0) continue;
    else if (pname.size() == 0 && isdigit(str.at(0))) {
      if (slot == 0)
        slot = atoi(str.c_str());
      else if (channel == 0)
        channel = atoi(str.c_str());
    } else if (_crate_m.find(str) != _crate_m.end()) {
      nodename = str;
    } else if (str == "SWITCH" || str == "RAMPUP_SPEED" ||
               str == "RAMPDOWN_SPEED" || str == "VOLTAGE_DEMAND" ||
               str == "VOLTAGE_LIMIT" || str == "CURRENT_LIMIT") {
      pname = str;
    } else if (pname.size() > 0) {
      value = str;
    } else {
      std::cout << "Unknown parameter : " << str << std::endl;
      std::cout << "Requested : set [<nodename> <slot> <channel>] <parame_name> <value>"
                << std::endl;
      return false;
    }
  }
  if (nodename.size() > 0) {
    if (_crate_m.find(nodename) == _crate_m.end()) {
      std::cout << "Unknown node : " << nodename << std::endl;
      std::cout << "Requested : set [<nodename> <slot> <channel>] <parame_name> <value>"
                << std::endl;
      return false;
    }
    return handle(set_local, _crate_m[nodename], slot, channel, pname, value);
  } else {
    for (HVCrateInfoMap::iterator it = _crate_m.begin();
         it != _crate_m.end(); it++) {
      if (!handle(set_local, it->second, slot, channel, pname, value)) {
        return false;
      }
    }
  }
  return true;
}

bool HVControlMaster::get(const std::string& s)
{
  std::vector<std::string> str_v = Belle2::split(s, ' ');
  size_t slot = 0;
  size_t channel = 0;
  std::string pname;
  std::string nodename;
  for (size_t i = 0; i < str_v.size(); i++) {
    std::string& str(str_v[i]);
    if (str.size() == 0) continue;
    else if (pname.size() == 0 && isdigit(str.at(0))) {
      if (slot == 0)
        slot = atoi(str.c_str());
      else if (channel == 0)
        channel = atoi(str.c_str());
    } else if (_crate_m.find(str) != _crate_m.end()) {
      nodename = str;
    } else if (str == "SWITCH" || str == "RAMPUP_SPEED" ||
               str == "RAMPDOWN_SPEED" || str == "VOLTAGE_DEMAND" ||
               str == "VOLTAGE_LIMIT" || str == "CURRENT_LIMIT") {
      pname = str;
    } else {
      std::cout << "Unknown parameter : " << str << std::endl;
      std::cout << "Requested : set [<nodename> <slot> <channel>] <parame_name> <value>"
                << std::endl;
      return false;
    }
  }
  if (nodename.size() > 0) {
    if (_crate_m.find(nodename) == _crate_m.end()) {
      std::cout << "Unknown node : " << nodename << std::endl;
      std::cout << "Requested : get [<nodename> <slot> <channel>] <parame_name>"
                << std::endl;
      return false;
    }
    return handle(get_local, _crate_m[nodename], slot, channel, pname, "");
  } else {
    for (HVCrateInfoMap::iterator it = _crate_m.begin();
         it != _crate_m.end(); it++) {
      if (!handle(get_local, it->second, slot, channel, pname, "")) {
        return false;
      }
    }
  }
  return true;
}

bool HVControlMaster::save(const std::string& nodename)
{
  ConfigFile config;
  _db.connect();
  if (nodename.size() == 0) {
    for (HVCrateInfoMap::iterator it = _crate_m.begin();
         it != _crate_m.end(); it++) {
      if (!save_local(it->first)) return false;
    }
  } else {
    if (!save_local(nodename)) return false;
  }
  _db.close();
  return true;
}

bool HVControlMaster::recall(const std::string& str)
{
  std::vector<std::string> str_v = Belle2::split(str, ' ');
  std::vector<std::string> nodename_v;
  if (str_v.size() > 0) nodename_v.push_back(str_v[0]);
  for (HVCrateInfoMap::iterator it = _crate_m.begin();
       it != _crate_m.end(); it++) {
    nodename_v.push_back(it->first);
  }
  for (size_t i = 0; i < nodename_v.size(); i++) {
    std::string nodename = nodename_v[i];
    if (_crate_m.find(nodename) == _crate_m.end()) {
      std::cout << "Unknown nodename : " << nodename << std::endl;
      return false;
    }
    int confno = (str_v.size() > 1) ? atoi(str_v[1].c_str()) : -1;
    HVCrateInfo* crate = _crate_m[nodename];
    _db.connect();
    if (confno <= 0) {
      try {
        _db.execute(Belle2::form("select confno from hv_database where node = '%s' order by confno desc limit 1;",
                                 nodename.c_str()));
        std::vector<DBRecord>& record_v(_db.loadRecords());
        if (record_v.size() > 0)
          confno = atoi(record_v[0].getFieldValue("confno").c_str());
      } catch (const IOException& e) {}
    }
    try {
      _db.execute(Belle2::form("select * from hv_database where node = '%s' and confno = %d;",
                               nodename.c_str(), confno));
      std::vector<DBRecord>& record_v(_db.loadRecords());
      if (record_v.size() == 0) {
        Belle2::debug("No entry found where node = %s and confno = %d", nodename.c_str(), confno);
      }
      for (size_t i = 0; i < record_v.size(); i++) {
        unsigned int slot = record_v[i].getFieldValueInt("slot");
        unsigned int ch = record_v[i].getFieldValueInt("channel");
        if (slot < crate->getNSlot() + 1 && ch < crate->getNChannel() + 1) {
          HVChannelInfo* info = crate->getChannelInfo(slot - 1, ch - 1);
          info->setValues(record_v[i].getFieldNames(), record_v[i].getFieldValues());
        }
      }
    } catch (const IOException& e) {}
    _db.close();
  }
  return true;
}

bool HVControlMaster::remove(const std::string& str)
{
  std::vector<std::string> str_v = Belle2::split(str, ' ');
  std::string nodename = (str_v.size() > 0) ? str_v[0] : "";
  if (nodename.size() > 0 &&
      _crate_m.find(nodename) == _crate_m.end()) {
    std::cout << "Unknown nodename : " << nodename << std::endl;
    return false;
  }
  std::string opt;
  if (nodename.size() > 0) {
    opt = Belle2::form("node = '%s'", nodename.c_str());
  }
  if (str_v.size() > 1) {
    if (opt.size() > 0) opt += " and ";
    opt += Belle2::form("confno = %d", atoi(str_v[1].c_str()));
  }
  if (opt.size() > 0) opt = "where " + opt;
  _db.connect();
  try {
    _db.execute(Belle2::form("delete from hv_database %s;", opt.c_str()));
  } catch (const IOException& e) {Belle2::debug("%s", e.what());}
  _db.close();
  return true;
}

bool HVControlMaster::list(const std::string& str)
{
  std::string nodename = str;
  std::string cmd = "";
  if (str.size() > 0) {
    cmd = "and node = '" + str + "'";
    if (_crate_m.find(nodename) == _crate_m.end()) {
      std::cout << "Unknown nodename : " << nodename << std::endl;
      return false;
    }
  }
  _db.connect();
  try {
    _db.execute(Belle2::form("select * from hv_database where slot = 1 and channel = 1 %s order by node;",
                             cmd.c_str()));
    std::vector<DBRecord>& record_v(_db.loadRecords());
    std::cout << "  node | confno " << std::endl;
    for (size_t i = 0; i < record_v.size(); i++) {
      std::cout << Belle2::form(" %5s | %6d",
                                record_v[i].getFieldValue("node").c_str(),
                                record_v[i].getFieldValueInt("confno")) << std::endl;
    }
  } catch (const IOException& e) {
    _db.close();
    return false;
  }
  _db.close();
  return true;
}

bool HVControlMaster::execute(const std::string& nodename,
                              HVCommand command)
{
  try {
    if (_comm != NULL && _comm->isOnline()) {
      if (nodename.size() > 0) {
        if (_node_m.find(nodename) != _node_m.end()) {
          _comm->sendRequest(_node_m[nodename], command);
        } else {
          Belle2::debug("Unknown nodename = %s ", nodename.c_str());
          return false;
        }
      } else {
        for (HVCrateInfoMap::iterator it = _crate_m.begin();
             it != _crate_m.end(); it++)
          _comm->sendRequest(_node_m[it->first], command);
      }
    }
  } catch (const IOException& e) {
    Belle2::debug("NSM error : %s ", e.what());
    return false;
  }
  return true;
}

bool HVControlMaster::handle(HVControlFunc_t func, HVCrateInfo* crate,
                             size_t slot, size_t channel,
                             const std::string& pname, const std::string& value)
{
  if (slot > crate->getNSlot()) {
    std::cout << "Too large slot number: " << slot
              << " > " << crate->getNSlot() << std::endl;
    return false;
  }
  if (channel > crate->getNChannel()) {
    std::cout << "Too large channel number: " << channel
              << " > " << crate->getNChannel() << std::endl;
    return false;
  }
  if (slot == 0) {
    for (size_t ns = 0; ns < crate->getNSlot(); ns++) {
      if (channel == 0) {
        for (size_t nc = 0; nc < crate->getNChannel(); nc++) {
          if (!func(crate->getChannelInfo(ns, nc),
                    crate->getChannelStatus(ns, nc), pname, value)) return false;
        }
      } else {
        if (!func(crate->getChannelInfo(ns, channel - 1),
                  crate->getChannelStatus(ns, channel - 1), pname, value)) return false;
      }
    }
  } else {
    if (channel == 0) {
      for (size_t nc = 0; nc < crate->getNChannel(); nc++) {
        if (!func(crate->getChannelInfo(slot - 1, nc),
                  crate->getChannelStatus(slot - 1, nc), pname, value)) return false;
      }
    } else {
      if (!func(crate->getChannelInfo(slot - 1, channel - 1),
                crate->getChannelStatus(slot - 1, channel - 1), pname, value)) return false;
    }
  }
  return true;
}

bool HVControlMaster::show_local(HVChannelInfo* info,
                                 HVChannelStatus* status,
                                 const std::string& pname,
                                 const std::string& value)
{

  if (value == "INFO") {
    std::cout << Belle2::form(" %5s | ", pname.c_str())
              << info->print_values() << std::endl;
  } else if (value == "STATUS") {
    std::cout << Belle2::form(" %5s | ", pname.c_str())
              << status->print_values() << std::endl;
  }
  return true;
}

bool HVControlMaster::set_local(HVChannelInfo* info,  HVChannelStatus*,
                                const std::string& pname,
                                const std::string& value)
{
  if (pname == "SWITCH") {
    info->setSwitchOn(value == "ON");
  } else if (pname == "RAMPUP_SPEED") {
    info->setRampUpSpeed(strtoul(value.c_str(), 0, 0));
  } else if (pname == "RAMPDOWN_SPEED") {
    info->setRampDownSpeed(strtoul(value.c_str(), 0, 0));
  } else if (pname == "VOLTAGE_DEMAND") {
    info->setVoltageDemand(strtoul(value.c_str(), 0, 0));
  } else if (pname == "VOLTAGE_LIMIT") {
    info->setVoltageLimit(strtoul(value.c_str(), 0, 0));
  } else if (pname == "CURRENT_LIMIT") {
    info->setCurrentLimit(strtoul(value.c_str(), 0, 0));
  } else {
    std::cout << "Unknown parmater: " << pname << std::endl;
    return false;
  }
  return true;
}

bool HVControlMaster::get_local(HVChannelInfo* info, HVChannelStatus*,
                                const std::string& pname_in,
                                const std::string&)
{
  std::string pname = Belle2::toupper(pname_in);
  if (pname == "SWITCH") {
    std::cout << info->isSwitchOn() << std::endl;
  } else if (pname == "RAMPUP_SPEED") {
    std::cout << info->getRampUpSpeed() << std::endl;
  } else if (pname == "RAMPDOWN_SPEED") {
    std::cout << info->getRampDownSpeed() << std::endl;
  } else if (pname == "VOLTAGE_DEMAND") {
    std::cout << info->getVoltageDemand() << std::endl;
  } else if (pname == "VOLTAGE_LIMIT") {
    std::cout << info->getVoltageLimit() << std::endl;
  } else if (pname == "CURRENT_LIMIT") {
    std::cout << info->getCurrentLimit() << std::endl;
  } else {
    std::cout << "Unknown parmater: " << pname << std::endl;
    return false;
  }
  return true;
}

bool HVControlMaster::save_local(const std::string& nodename)
{
  HVCrateInfo* crate = _crate_m[nodename];
  try {
    _db.execute(Belle2::form("create table hv_database (node text, %s);",
                             crate->getChannelInfo(0, 0)->toSQLConfig().c_str()));
  } catch (const IOException& e) {}
  int confno = 1;
  try {
    _db.execute(Belle2::form("select confno from hv_database where node = '%s' order by confno desc limit 1;",
                             nodename.c_str()));
    std::vector<DBRecord>& record_v(_db.loadRecords());
    if (record_v.size() > 0)
      confno = atoi(record_v[0].getFieldValue("confno").c_str()) + 1;
  } catch (const IOException& e) {}
  try {
    for (size_t ns = 0; ns < crate->getNSlot(); ns++) {
      for (size_t nc = 0; nc < crate->getNChannel(); nc++) {
        HVChannelInfo* info = crate->getChannelInfo(ns, nc);
        info->setConfigNumber(confno);
        _db.execute(Belle2::form("insert into hv_database (node, %s) values ('%s', %s);",
                                 info->toSQLNames().c_str(),
                                 nodename.c_str(), info->toSQLValues().c_str()));
      }
    }
  } catch (const IOException& e) {
    std::cout << "DB Fatal error: " << e.what() << std::endl;
    _db.close();
    return false;
  }
  std::cout << "Save for " << nodename
            << " with configuration number = " << confno << std::endl;
  return true;
}

