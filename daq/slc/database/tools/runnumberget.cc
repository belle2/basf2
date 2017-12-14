#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/RunNumberTable.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Date.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  int expno = 0;
  int runno_min = 0;
  int runno_max = -1;
  std::string type;
  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "-h") {
      printf("usage: %s [-exp <expno>] [-runmin <min>] [-runmax <max>\n",
             argv[0]);
      return 1;
    }
    if (std::string(argv[i]) == "-exp") {
      i++;
      expno = atoi(argv[i]);
    }
    if (std::string(argv[i]) == "-runmin") {
      i++;
      runno_min = atoi(argv[i]);
    }
    if (std::string(argv[i]) == "-runmax") {
      i++;
      runno_max = atoi(argv[i]);
    }
    if (std::string(argv[i]) == "-type") {
      i++;
      type = argv[i];
    }
  }
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  db.connect();

  const RunNumberList list(type.size() > 0 ?
                           RunNumberTable(db).get(type, expno, runno_min, runno_max) :
                           RunNumberTable(db).get(expno, runno_min, runno_max));
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
