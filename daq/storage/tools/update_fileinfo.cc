#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/statvfs.h>

using namespace Belle2;

const char* g_table = "fileinfo";

int main(int argc, char** argv)
{
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  //  id  |             name             |                         path                         |         host          | label  | expno | runno | fileno |    size    | nevents |   chksum   |       time_close
  //    | time_sent | time_remove
  try {
    db.connect();
    db.execute("select * from datafiles where path like '/rawdata/disk01/storage/%s.%_' order by time_close;", argv[1]);
    DBRecordList record_v(db.loadRecords());
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      std::string name = record.get("name");
      std::string path = record.get("path");
      std::string host = "HLT1";
      std::string label = record.get("label");
      unsigned int id = record.getInt("id");
      unsigned int expno = record.getInt("expno");
      unsigned int runno = record.getInt("runno");
      unsigned int fileno = record.getInt("fileno");
      int diskid = 1;
      std::string dir = "/rawdata/disk";
      std::string filedir = dir + StringUtil::form("%02d/storage/%4.4d/%5.5d/",
                                                   diskid, expno, runno);
      name = StringUtil::form("%s.%s.%4.4d.%5.5d.f%5.5d.sroot", host.c_str(),
                              label.c_str(), expno, runno, fileno);
      std::string path_out = filedir + name;
      std::cout << "mkdir -p " << filedir << std::endl;
      system(("mkdir -p " + filedir).c_str());
      std::cout << "mv " << path << " " << path_out << std::endl;
      system((("mv " + path) + (" " + path_out)).c_str());
      db.execute("update datafiles set name = '%s', path = '%s' where id = %d",
                 name.c_str(), path_out.c_str(), id);
    }
    //db.execute(ss.str().c_str());
    db.close();
  } catch (const DBHandlerException& e) {
    LogFile::error("Failed to update : %s", e.what());
    return 1;
  }
  return 0;
}

