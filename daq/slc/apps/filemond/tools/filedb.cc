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
#include <sstream>
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
  nevents--;
  return size;
}

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    const int m_runno_start = config.getInt("start.runno");
    const int m_expno_start = config.getInt("start.expno");
    const std::string m_table = config.get("table");
    const std::string m_host = config.get("host");
    const std::string m_list_send = config.get("list.send");
    const std::string m_list_sent = config.get("list.sent");
    const std::string m_label_enabled = config.get("label.enabled");
    const std::string m_homedir = config.get("homedir");
    const std::vector<std::string> m_dirs = StringUtil::split(config.get("dirs"), ',');

    PostgreSQLInterface db(config.get("database.host"),
                           config.get("database.dbname"),
                           config.get("database.user"),
                           config.get("database.password"),
                           config.getInt("database.port"));

    while (true) {
      try {
        std::string d = Date().toString();
        for (std::vector<std::string>::const_iterator it = m_dirs.begin();
             it != m_dirs.end(); it++) {
          std::string dir = *it;
          std::string file = m_list_sent + dir;
          LogFile::info("sent list : " + file);
          if (File::exist(file.c_str())) {
            std::ifstream fin(file.c_str());
            int count = 0;
            std::string s;
            while (fin && getline(fin, s)) {
              StringList ss = StringUtil::split(s, ',');
              std::string path = ss[0];
              ss = StringUtil::split(path, '/');
              std::string name = ss[ss.size() - 1];
              name = StringUtil::replace(name, ".root", ".sroot");
              db.connect();
              db.execute("update %s set time_sent='%s' where label != 'daq' and host='%s' and name='%s' and time_sent is null returning id;",
                         m_table.c_str(), d.c_str(), m_host.c_str(), name.c_str());
              LogFile::debug("update %s set time_sent='%s' where label != 'daq' and host='%s' and name='%s' and time_sent is null returning id;",
                             m_table.c_str(), d.c_str(), m_host.c_str(), name.c_str());
              DBRecordList records(db.loadRecords());
              if (records.size() > 0) {
                LogFile::info("Done " + name);
                count++;
              }
            }
            file = m_homedir + dir + "/storage/" + m_list_send;
            if (count > 0 && File::exist(file.c_str())) {
              std::string newfile = file + StringUtil::form("%d", Date().get());
              LogFile::info("renmame file : %s to %s ", file.c_str(), newfile.c_str());
              ::rename(file.c_str(), newfile.c_str());
            }
          }
        }
      } catch (const DBHandlerException& e) {
        LogFile::fatal(e.what());
        return 1;
      }

      int expno, runno;
      try {
        db.connect();
        db.execute("select * from runnumber where isstart = false and expno >= %d and runno >= %d order by id desc limit 1;", m_expno_start,
                   m_runno_start);
        //LogFile::debug("select * from runnumber where isstart = false and expno >= %d and runno >= %d order by id desc limit 1;", m_expno_start, m_runno_start);
        DBRecordList records(db.loadRecords());
        if (records.size() > 0) {
          expno = records[0].getInt("expno");
          runno = records[0].getInt("runno");
          LogFile::debug("expno=%04d.%05d", expno, runno);
        } else {
          sleep(60);
          continue;
        }
      } catch (const DBHandlerException& e) {
        LogFile::fatal(e.what());
        return 1;
      }

      try {
        for (size_t j = 0; j < m_dirs.size(); j++) {
          std::string dir = m_homedir + m_dirs[j] + "/storage/";
          db.execute("select * from %s where label != 'daq' and host = '%s' and time_sent is null "
                     "and (expno < %d or (expno = %d and runno <= %d)) and expno >= %d and runno >= %d "
                     "and path like '%s%s' order by time_close ",
                     m_table.c_str(), m_host.c_str(), expno, expno, runno, m_expno_start, m_runno_start, dir.c_str(), "_%");
          //LogFile::debug("select * from %s where label != 'daq' and host = '%s' and time_sent is null "
          //           "and (expno < %d or (expno = %d and runno <= %d)) and expno >= %d and runno >= %d "
          //       "and path like '%s%s' order by time_close ",
          //           m_table.c_str(), m_host.c_str(), expno, expno, runno, m_expno_start, m_runno_start, dir.c_str(), "_%");
          DBRecordList records(db.loadRecords());
          struct exp_run {
            int exp;
            int run;
            std::string label;
          };
          std::vector<exp_run> masks;
          for (size_t i = 0; i < records.size(); i++) {
            int expno = records[i].getInt("expno");
            int runno = records[i].getInt("runno");
            std::string label = records[i].get("label");
            std::string time_close = records[i].get("time_close");
            if (time_close.size() == 0) {
              exp_run mask = {expno, runno, label};
              masks.push_back(mask);
            }
          }
          unsigned int nfiles = records.size();
          for (size_t i = 0; i < records.size(); i++) {
            std::string name = records[i].get("name");
            int expno = records[i].getInt("expno");
            int runno = records[i].getInt("runno");
            std::string label = records[i].get("label");
            for (size_t j = 0; j < masks.size(); j++) {
              if (label == masks[j].label && expno == masks[j].exp && runno == masks[j].run) {
                nfiles--;
                continue;
              }
            }
          }
          std::string file_tmp = dir + m_list_send + ".tmp";
          std::string file = dir + m_list_send;
          //LogFile::info("creating send list " + file);
          //std::string file = dir+StringUtil::form("%s.disk%02d", m_list_send.c_str(), j+1);
          int count = 0;
          if (nfiles > 0 && !File::exist(file.c_str())) {
            LogFile::info("created send list " + file_tmp);
            LogFile::info("select files expno==%d, runno<=%d and runno >= %d", expno, runno, m_runno_start);
            std::ofstream fout(file_tmp.c_str());
            for (size_t i = 0; i < records.size(); i++) {
              std::string name = records[i].get("name");
              int expno = records[i].getInt("expno");
              int runno = records[i].getInt("runno");
              std::string label = records[i].get("label");
              bool masked = false;
              for (size_t j = 0; j < masks.size(); j++) {
                if (label == masks[j].label && expno == masks[j].exp && runno == masks[j].run) {
                  masked = true;
                }
              }
              if (masked) {
                LogFile::info("Masked " + name);
                continue;
              }
              std::string path = records[i].get("path");
              std::string host = records[i].get("host");
              int fileno = records[i].getInt("fileno");
              unsigned long long size = records[i].getInt("size");
              unsigned long long nevents = records[i].getInt("nevents");
              unsigned long long chksum = records[i].getInt("chksum");
              ///*
              struct stat st;
              stat(path.c_str(), &st);
              if (size != st.st_size) {
                std::string d = Date(st.st_mtime).toString();
                unsigned long long chksum_new, nevents_new, size_new;
                size_new = cal_chksum(path.c_str(), chksum_new, nevents_new);
                if (fileno == 0) nevents--;
                db.execute("update %s set time_close='%s', chksum=%lu, nevents=%lu, size=%lu "
                           "where name='%s' and host='%s';",
                           m_table.c_str(), d.c_str(), chksum_new, nevents_new, size_new,
                           name.c_str(), m_host.c_str());
                LogFile::info("update file: %s (size: %lu>>%lu, nevents: %lu>>%lu, chksum: %lu>>%lu)", name.c_str(),
                              size, size_new, nevents, nevents_new, chksum, chksum_new);
                chksum = chksum_new;
                size = size_new;
                nevents = nevents_new;
              }
              //*/
              if (m_label_enabled.find(label) != std::string::npos) {
                std::string s_chksum = StringUtil::form("%x", chksum);
                fout << StringUtil::form("%4.4d/%5.5d/", expno, runno) << name << "," << expno << "," << runno << "," << fileno << ","
                     << size << "," << nevents << "," << s_chksum << "" << std::endl;
                std::stringstream ss;
                ss << StringUtil::form("%4.4d/%5.5d/", expno, runno) << name << "," << expno << "," << runno << "," << fileno << ","
                   << size << "," << nevents << "," << s_chksum << "";
                count++;
                LogFile::info(ss.str());
              }
            }
            fout.close();
            if (count > 0) {
              ::rename(file_tmp.c_str(), file.c_str());
              LogFile::info("renamed " + file_tmp + " to " + file);
            }
          }
        }
      } catch (const DBHandlerException& e) {
        LogFile::fatal(e.what());
        return 1;
      }
      sleep(60);
    }
  }

  return 0;
}
