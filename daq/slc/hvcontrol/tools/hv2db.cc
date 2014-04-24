#include "daq/slc/hvcontrol/HVCommand.h"
#include "daq/slc/hvcontrol/HVState.h"
#include "daq/slc/hvcontrol/hv_status.h"

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/database/ConfigObjectTable.h>
#include <daq/slc/database/ConfigInfoTable.h>
#include <daq/slc/database/LoggerObjectTable.h>
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

void nsmget(const std::string& hvnodename, hv_status* status)
{
  HVState s(status->state);
  std::stringstream ss1, ss;
  std::string l = StringUtil::form(" index | %13s | %7s | %7s",
                                   "state", "voltage", "current");
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
                           StringUtil::form("%4.1f", ch_status.current_mon).c_str())
       << std::endl;
  }
  ss << ss1.str();
  std::cout << "Updated at " << Date().toString() << std::endl
            << ss.str();
}

void dbget(const std::string& configname,
           const std::string& hvnodename,
           DBInterface* db, ConfigObject& obj)
{
  ConfigObjectTable table(db);
  db->connect();
  obj = table.get(configname, hvnodename);
  db->close();
}

class NSMListener {

public:
  NSMListener(NSMCommunicator* comm, NSMNode& node,
              NSMNode& hvnode, NSMData& data, DBInterface* db)
    : m_comm(comm), m_node(node), m_hvnode(hvnode),
      m_data(data), m_db(db) {}
  virtual ~NSMListener() throw() {}

public:
  void run() {
    while (true) {
      fputs("\033[2J\033[0;0H", stdout);
      rewind(stdout);
      ftruncate(1, 0);
      nsmget(m_hvnode.getName(), (hv_status*)m_data.get());
      //m_db->connect();
      //LoggerObjectTable(m_db).add(m_data, true);
      //m_db->close();
      sleep(5);
    }
  }

private:
  NSMCommunicator* m_comm;
  NSMNode& m_node;
  NSMNode& m_hvnode;
  NSMData& m_data;
  DBInterface* m_db;

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
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  db->connect();
  ConfigInfo cinfo = ConfigInfoTable(db).get(status->configid);
  ConfigObjectTable table(db);
  ConfigObject obj = table.get(cinfo.getName(), hvnodename);
  //LoggerObjectTable(db).add(data);
  db->close();

  NSMListener(comm, node, hvnode, data, db).run();
  return 0;
}

