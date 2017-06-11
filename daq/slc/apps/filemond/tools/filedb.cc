#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DAQLogMessage.h>

#include <daq/slc/system/Inotify.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/File.h>
#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/Date.h>
#include <daq/slc/base/ConfigFile.h>

#include <map>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <algorithm>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <stdio.h>
#include <stdint.h>
#include "zlib.h"

#define IOSIZE  (100 * 1024 * 1024)
unsigned int* buf = new unsigned int[IOSIZE];

using namespace Belle2;

unsigned long long cal_chksum(const char* filename, unsigned long long& chksum, unsigned long long& nevents)
{
  chksum = 1;
  FILE* fp = fopen(filename, "r");
  unsigned int size = 0;
  nevents = 0;
  while (true) {
    unsigned int len;
    len = fread(buf, 1, sizeof(unsigned int), fp);
    if (len == 0)
      break;
    nevents++;
    len = fread(buf + 1, 1, buf[0] - sizeof(int), fp);
    if (len == 0)
      break;
    len += sizeof(int);
    chksum = adler32(chksum, (unsigned char*)buf, len);
    size += len;
  }
  LogFile::info("%s size=%d nevent=%d chksum=%08x", filename, size, nevents, chksum);
  fclose(fp);
  return size;
}

const char* g_lockfile = "/tmp/filedb";

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    {
      std::ifstream fin(g_lockfile);
      int flag;
      fin >> flag;
      if (flag == 1) {
        LogFile::debug("other filedb is still running");
        return 0;
      }
      std::ofstream fout(g_lockfile);
      fout << 1;
    }

    ConfigFile config("slowcontrol", argv[1]);
    const std::string m_table = config.get("table");
    const std::string m_host = config.get("host");
    const std::string m_list_send = config.get("list.send");
    const std::string m_list_sent = config.get("list.sent");

    PostgreSQLInterface db(config.get("database.host"),
                           config.get("database.dbname"),
                           config.get("database.user"),
                           config.get("database.password"),
                           config.getInt("database.port"));
    try {
      std::string d = Date().toString();
      std::string path;
      std::ifstream fin(m_list_sent.c_str());
      while (fin >> path) {
        db.connect();
        db.execute("update %s set time_sent='%s' where host='%s' and path='%s';",
                   m_table.c_str(), d.c_str(), m_host.c_str(), path.c_str());
      }
    } catch (const DBHandlerException& e) {
      LogFile::fatal(e.what());
      unlink(g_lockfile);
      return 1;
    }

    int expno, runno;
    try {
      db.connect();
      db.execute("select * from runnumber where isstart = false order by id desc limit 1;");
      DBRecordList records(db.loadRecords());
      if (records.size() > 0) {
        expno = records[0].getInt("expno");
        runno = records[0].getInt("runno");
      } else {
        unlink(g_lockfile);
        return 0;
      }
    } catch (const DBHandlerException& e) {
      LogFile::fatal(e.what());
      unlink(g_lockfile);
      return 1;
    }

    try {
      const std::vector<std::string> dirs = StringUtil::split(config.get("dirs"), ',');
      db.connect();
      db.execute("select * from %s where host = '%s' "
                 "and time_close is null and (expno <=%d or runno <= %d)",
                 m_table.c_str(), m_host.c_str(), expno, runno);
      std::map<std::string, int> names;
      DBRecordList records(db.loadRecords());
      if (records.size() > 0) {
        for (size_t i = 0; i < records.size(); i++) {
          names.insert(std::pair<std::string, int>(records[i].get("name"),
                                                   records[1].getInt("fileno")));
        }
        int nrecords = records.size();
        for (std::vector<std::string>::const_iterator it = dirs.begin();
             it != dirs.end(); it++) {
          std::string dir = *it;
          DIR* dp = opendir(dir.c_str());
          struct dirent* dirst;
          while ((dirst = readdir(dp)) != NULL) {
            std::string name = dirst->d_name;
            if (name.find(".sroot") != std::string::npos && name.find(".gz") == std::string::npos) {
              std::string filepath = dir + "/" + name;
              std::map<std::string, int>::iterator it = names.find(name);
              if (it != names.end()) {
                while (true) {
                  struct stat st;
                  stat(filepath.c_str(), &st);
                  time_t t;
                  time(&t);
                  if ((t - st.st_mtime) > 60) {
                    std::string d = Date(st.st_mtime).toString();
                    unsigned long long chksum, nevents, size;
                    size = cal_chksum(filepath.c_str(), chksum, nevents);
                    if (it->second == 0) nevents--;
                    db.execute("update %s set time_close='%s', chksum=%lu, nevents=%lu, size=%lu "
                               "where name='%s' and host='%s';",
                               m_table.c_str(), d.c_str(), chksum, nevents, size,
                               name.c_str(), m_host.c_str());
                    LogFile::info("new file: %s (%s)", name.c_str(), d.c_str());
                    nrecords--;
                    break;
                  }
                  sleep(t - st.st_mtime + 5);
                }
              }
            }
            closedir(dp);
          }
        }
      }
    } catch (const DBHandlerException& e) {
      LogFile::fatal(e.what());
      unlink(g_lockfile);
      return 1;
    }

    try {
      db.execute("select * from %s where host = '%s' and time_sent is null "
                 "and (expno <=%d or runno <= %d) order by time_close ",
                 m_table.c_str(), m_host.c_str(), expno, runno);
      DBRecordList records(db.loadRecords());
      if (records.size() > 0 && !File::exist(m_list_send.c_str())) {
        std::ofstream fout(m_list_send.c_str());
        for (size_t i = 0; i < records.size(); i++) {
          std::string path = records[i].get("path");
          std::string expno = records[i].get("expno");
          std::string runno = records[i].get("runno");
          std::string fileno = records[i].get("fileno");
          std::string size = records[i].get("size");
          std::string nevents = records[i].get("nevents");
          std::string chksum = records[i].get("chksum");
          fout << path << "," << expno << "," << runno << "," << fileno << ","
               << size << "," << nevents << "," << chksum << "" << std::endl;
        }
        fout.close();
      }
    } catch (const DBHandlerException& e) {
      LogFile::fatal(e.what());
      unlink(g_lockfile);
      return 1;
    }
    unlink(g_lockfile);
  }

  return 0;
}
