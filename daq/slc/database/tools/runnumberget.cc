#include <daq/slc/database/PostgreSQLInterface.h>
#include <daq/slc/database/RunNumberTable.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Date.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 1 || (argc > 1 && std::string(argv[1]) == "-h")) {
    printf("usage: %s [<expno>]\n", argv[0]);
    return 1;
  }
  int expno = (argc > 1) ? atoi(argv[1]) : 0;
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  db.connect();
  const RunNumberList list(RunNumberTable(db).get(expno));
  for (RunNumberList::const_iterator it = list.begin();
       it != list.end(); it++) {
    const RunNumber& rn(*it);
    printf("[%s] %03d.%05d.%03d=%s %s\n",
           Date(rn.getRecordTime()).toString(),
           rn.getExpNumber(), rn.getRunNumber(), rn.getSubNumber(),
           rn.getConfig().c_str(), (rn.isStart() ? "start" : "end"));
  }
  return 0;
}
