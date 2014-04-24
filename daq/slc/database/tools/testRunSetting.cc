#include <daq/slc/database/ConfigObjectTable.h>
#include <daq/slc/database/LoggerObjectTable.h>
#include <daq/slc/database/RunNumberInfoTable.h>

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <unistd.h>

namespace Belle2 {

  class RunSetting : public ConfigObject {

  public:
    enum Cause {
      MANUAL = 1, AUTO, RUNEND, ERROR, ERROR_MANUAL
    };

  public:
    RunSetting() {
      setConfig(false);
      setTable("runsetting");
      setRevision(1);
      addInt("runnumberid", 0);
      addEnumList("cause", "manual,auto,runend,error,error_manual");
      addEnum("cause", "manual");
      addText("operators", "");
      addText("comment", "");
      addObject("runcontrol", ConfigObject());
    }
    ~RunSetting() throw() {}

  public:
    void setRunNumber(RunNumberInfo info) { setInt("runnumberid", info.getId()); }
    void setCause(Cause cause) {
      switch (cause) {
        case MANUAL: setEnum("cause", "manual"); break;
        case AUTO: setEnum("cause", "auto"); break;
        case RUNEND: setEnum("cause", "runend"); break;
        case ERROR: setEnum("cause", "error"); break;
        case ERROR_MANUAL: setEnum("cause", "error_manual"); break;
      }
    }
    void setOperators(const std::string& operators) { setText("operators", operators); }
    void setComment(const std::string& comment) { setText("comment", comment); }
    void setRunControl(const ConfigObject& obj) { setObject("runcontrol", 0, obj); }

  };

}

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage: dbtest5 <nodename> [<configname>]");
    return 1;
  }
  const std::string nodename = argv[1];
  const std::string configname = (argc > 2) ? argv[2] : "default";

  ConfigFile dbconfig("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(dbconfig.get("database.host"),
                                            dbconfig.get("database.dbname"),
                                            dbconfig.get("database.user"),
                                            dbconfig.get("database.password"),
                                            dbconfig.getInt("database.port"));
  db->connect();
  ConfigObject obj = ConfigObjectTable(db).get(configname, nodename);
  RunSetting setting;
  setting.setOperators("KONNO:TOMOYUKI");
  setting.setRunControl(obj);
  setting.setCause(RunSetting::MANUAL);
  LoggerObjectTable(db).add(setting);
  while (true) {
    sleep(10);
  }
  return 0;
}
