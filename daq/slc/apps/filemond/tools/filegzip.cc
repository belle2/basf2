#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DAQLogMessage.h>

#include <daq/slc/system/Inotify.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/File.h>

#include <daq/slc/base/Date.h>
#include <daq/slc/base/ConfigFile.h>

#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <algorithm>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

using namespace Belle2;

time_t toUnixtime(const char* str)
{
  struct tm t;
  sscanf(str, "%d:%d:%d",
         &t.tm_hour, &t.tm_min, &t.tm_sec);
  Date d;
  t.tm_year =  d.getYear() - 1900;
  t.tm_mon = d.getMonth() - 1;
  t.tm_mday = d.getDay();
  return mktime(&t);
}

typedef std::pair<std::string, std::string> fileinfo_t;
bool sort_less(const fileinfo_t& left, const fileinfo_t& right)
{
  return left.second < right.second;
}

const char* fileout_path = "/x02/disk01/list_send.txt";
const char* filein_path = "/x02/disk01/list_sent.txt";

int main()
{
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  db.connect();
  while (true) {
    db.execute("select * from datafiles where time_gzip is null and id > 5487 order by time_close;");
    DBRecordList records(db.loadRecords());
    for (size_t i = 0; i < records.size(); i++) {
      std::string name = records[i].get("name");
      LogFile::debug(name);
      LogFile::debug(StringUtil::form("/x02/disk01/srootfiles/%s", name.c_str()));
      LogFile::debug(StringUtil::form("/x02/disk01/gzfiles2/%s.gz", name.c_str()));
      if (File::exist(StringUtil::form("/x02/disk01/srootfiles/%s", name.c_str()))
          && !File::exist(StringUtil::form("/x02/disk01/gzfiles2/%s.gz", name.c_str()))) {
        std::stringstream ss;
        ss << "gzip -c /x02/disk01/srootfiles/" << name << " > /x02/disk01/gzfiles2/" << name << ".gz" ;
        std::string cmd = ss.str();
        LogFile::info(cmd);
        system(cmd.c_str());
        if (File::exist(StringUtil::form("/x02/disk01/gzfiles2/%s.gz", name.c_str()))) {
          std::string d = Date().toString();
          try {
            db.execute("update datafiles set time_gzip = '%s' where name = '%s' and time_sent is null;",
                       d.c_str(), name.c_str());
          } catch (const DBHandlerException& e) {

          }
        }
      }
    }
    sleep(60);
  }

  return 0;
}
