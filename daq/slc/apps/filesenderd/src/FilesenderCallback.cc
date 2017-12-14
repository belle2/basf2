#include "daq/slc/apps/filesenderd/FilesenderCallback.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdio>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <stdlib.h>

using namespace Belle2;

FilesenderCallback::FilesenderCallback(const std::string& name, int expno, int runno,
                                       const std::string& path_ipnp, const std::string& path_pxd,
                                       const std::string& dirs, const std::string& excludes,
                                       DBInterface& db)
  : NSMCallback(10), m_db(db)
{
  setNode(NSMNode(name));
  setTimeout(10);
  m_expno = expno;
  m_runno = runno;
  m_path_ipnp = path_ipnp;
  m_path_pxd = path_pxd;
  m_dirs = StringUtil::split(dirs, ',');
  m_excludes = StringUtil::split(excludes, ',');
}

FilesenderCallback::~FilesenderCallback() throw()
{
}

void FilesenderCallback::init(NSMCommunicator& com) throw()
{
  timeout(com);
  /*
  for (std::vector<std::string>::iterator it = m_dirs.begin(); it != m_dirs.end(); it++) {
    const std::string& dir(*it);
    DIR* dp = opendir(dir.c_str());
    struct dirent* dirst;
    while ((dirst = readdir(dp)) != NULL) {
      std::string filename = dirst->d_name;
      if (filename.find(".sroot") != std::string::npos || filename.find(".root") != std::string::npos) {
  bool excluded = false;
  for (size_t i = 0; i < m_excludes.size(); i++) {
    if (m_excludes[i] == filename) {
      excluded = true;
    }
  }
  if (!excluded) {
    struct stat st;
    std::string filepath = dir + "/" + filename;
    stat(filepath.c_str(), &st);
    unsigned int t = st.st_mtim.tv_sec;
    LogFile::debug("Found %s (modified at %s)", filepath.c_str(), Date(t).toString());
  }
      }
    }
    closedir(dp);
  }
  */
}

void FilesenderCallback::timeout(NSMCommunicator&) throw()
{
  m_db.connect();
  m_db.execute("select * from filesender where expno >= %d and runno > %d ", m_expno, m_runno);
  DBRecordList records(m_db.loadRecords());
  m_db.close();
  unsigned int t1 = Time().get();
  for (std::vector<std::string>::iterator it = m_dirs.begin(); it != m_dirs.end(); it++) {
    const std::string& dir(*it);
    DIR* dp = opendir(dir.c_str());
    struct dirent* dirst;
    while ((dirst = readdir(dp)) != NULL) {
      std::string filename = dirst->d_name;
      std::string filepath = dir + "/" + filename;
      if (filename.find(".sroot") != std::string::npos || filename.find(".root") != std::string::npos) {
        StringList ss = StringUtil::split(filename, '.');
        if (ss.size() < 4) continue;
        std::string exe = ss[ss.size() - 1];
        bool is_sroot = (exe.find("sroot") != std::string::npos);
        int expno = atoi(ss[ss.size() - 3].c_str());
        int runno = atoi(ss[ss.size() - 2].c_str());
        int fileno = (exe.find("sroot-") != std::string::npos) ? atoi(StringUtil::replace(exe, "sroot-", "").c_str()) : 0;
        bool excluded = false;
        for (size_t i = 0; excluded && i < m_excludes.size(); i++) {
          if (m_excludes[i] == filename) {
            excluded = true;
          }
        }
        if (excluded) continue;
        bool found = false;
        std::string time_pxd, time_ipnp;
        for (size_t i = 0; !found && i < records.size(); i++) {
          DBRecord& record(records[i]);
          if (record.get("path_rpc2") == filepath) {
            found = true;
            time_pxd = record.get("time_pxd");
            time_ipnp = record.get("time_ipnp");
            break;
          }
        }
        if ((time_pxd.size() == 0 || time_ipnp.size() == 0) &&
            expno > 0 && runno > 0 && expno >= m_expno && runno > m_runno/* &&
                       (m_files.find(filename) == m_files.end() || m_files[filename] == 0)*/) {
          struct stat st;
          stat(filepath.c_str(), &st);
          unsigned int t = st.st_mtim.tv_sec;
          if (!found) {
            m_db.connect();
            m_db.execute("insert into filesender (path_rpc2, expno, runno, fileno, time_close) values ('%s', %d, %d, %d, '%s')",
                         filepath.c_str(), expno, runno, fileno, Date(t).toString());
            m_db.close();
          }
          m_files.insert(std::map<std::string, int>::value_type(filename, t));
          if (t1 - t > 5 * 60) {
            std::stringstream s;
            std::string path;
            int st;
            if (time_pxd.size() == 0) {
              s << m_path_pxd << (is_sroot ? "/srootfiles/" : "/rootfiles/");
              path = s.str();
              s.str("");
              s << "scp -oStrictHostKeyChecking=no -C -p -B -i /x02/disk01/scripts/.id_rsa " << filepath
                << " b2daq@belle-pxd:" << path << std::endl;
              LogFile::debug("Start transfer %s to belle-pxd:%s (modified at %s)", filepath.c_str(), path.c_str(), Date(t).toString());
              st = system(s.str().c_str());
              t1 = Time().get();
              if (WIFEXITED(st)) {
                LogFile::info("Succeded transfer belle-pxd:%s to %s", filepath.c_str(), path.c_str());
                m_db.connect();
                m_db.execute("update filesender set path_pxd = '%s', time_pxd = current_timestamp where path_rpc2 = '%s';",
                             (path + filename).c_str(), filepath.c_str());
                m_db.close();
              } else {
                LogFile::warning("Failed transfer %s to belle-pxd:%s", filepath.c_str(), path.c_str());
              }
            }
            if (time_ipnp.size() == 0) {
              s.str("");
              s << m_path_ipnp << (is_sroot ? "/srootfiles/" : "/rootfiles/");
              path = s.str();
              s.str("");
              s << "scp -oStrictHostKeyChecking=no -C -p -B -i /x02/disk01/scripts/.id_rsa -P 20022 " << filepath
                << " tkonno@localhost:" << path << std::endl;
              LogFile::debug(" Start transfer %s to ipnp30:%s (modified at %s)", filepath.c_str(), path.c_str(), Date(t).toString());
              st = system(s.str().c_str());
              t1 = Time().get();
              if (WIFEXITED(st)) {
                LogFile::info("Succeded transfer %s to ipnp30:%s", filepath.c_str(), path.c_str());
                m_db.connect();
                m_db.execute("update filesender set path_ipnp = '%s', time_ipnp = current_timestamp where path_rpc2 = '%s';",
                             (path + filename).c_str(), filepath.c_str());
                m_db.close();
              } else {
                LogFile::warning("Failed transfer %s to ipnp30:%s", filepath.c_str(), path.c_str());
              }
            }
          }
        }
      }
    }
    closedir(dp);
  }
}

