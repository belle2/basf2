//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <framework/logging/Logger.h>
#include <framework/pcore/SeqFile.h>

#include <daq/storage/BinData.h>
#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <fstream>
#include <queue>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/statvfs.h>

using namespace Belle2;

const unsigned long long GB = 1024 * 1024 * 1024;
const unsigned long long MAX_FILE_SIZE = 2 * GB;
const char* g_table = "fileinfo";

std::string popen(const std::string& cmd)
{
  char buf[1000];
  FILE* fp;
  if ((fp = ::popen(cmd.c_str(), "r")) == NULL) {
    perror("can not exec commad");
    exit(EXIT_FAILURE);
  }
  std::stringstream ss;
  while (!feof(fp)) {
    memset(buf, 0, 1000);
    fgets(buf, sizeof(buf), fp);
    ss << buf << std::endl;
  }
  pclose(fp);
  return ss.str();
}

class FileHandler {

public:
  FileHandler(DBInterface& db, const std::string& runtype,
              const char* host, const char* dbtmp)
    : m_db(db), m_runtype(runtype), m_host(host), m_dbtmp(dbtmp)
  {
    m_file = -1;
    m_filesize = 0;
  }
  ~FileHandler() throw()
  {
  }

public:
  int getDiskId() { return m_diskid; }
  int getFileId() { return m_fileid; }

public:
  int open(const std::string& dir, int ndisks, int expno, int runno)
  {
    m_fileid = 0;
    m_diskid = 1;
    try {
      m_db.connect();
      m_db.execute("select fileid, diskid from %s where expno = %d and runno = %d "
                   "and path_sroot like '%s:_%s' and runtype='%s' order by id desc;",
                   g_table, expno, runno, m_host.c_str(), "%%", m_runtype.c_str());
      DBRecordList record_v(m_db.loadRecords());
      if (record_v.size() > 0) {
        DBRecord& record(record_v[0]);
        m_fileid = (record.hasField("fileid")) ? record.getInt("fileid") + 1 : 0;
        m_diskid = (record.hasField("diskid")) ? record.getInt("diskid") : 1;
        std::cout << "db: fileid: " << m_fileid << std::endl;
      }
      m_db.close();
    } catch (const DBHandlerException& e) {
      LogFile::error("Failed to access db for read: %s", e.what());
      return -1;
    }
    std::cout << "fileid: " << m_fileid << std::endl;
    char filename[1024];
    bool available = false;
    for (int i = 0; i < ndisks; i++) {
      struct statvfs statfs;
      sprintf(filename, "%s%02d", dir.c_str(), m_diskid);
      statvfs(filename, &statfs);
      float usage = 1 - ((float)statfs.f_bfree / statfs.f_blocks);
      if (usage < 0.9) {
        sprintf(filename, "%s%02d/storage/full_flag", dir.c_str(), m_diskid);
        std::ifstream fin(filename);
        int flag = 0;
        fin >> flag;
        if (/*fin.eof() || */flag != 1) {
          available = true;
          B2INFO("disk : " << m_diskid << " is available");
          break;
        }
        fin.close();
        B2INFO("disk : " << m_diskid << " is still full");
      } else {
        sprintf(filename, "%s%02d/storage/full_flag", dir.c_str(), m_diskid);
        std::ofstream fout(filename);
        fout << 1;
        fout.close();
        B2WARNING("disk : " << m_diskid << " is full " << usage);
      }
      m_diskid++;
      if (m_diskid > ndisks) m_diskid = 1;
    }
    if (!available) B2FATAL("No disk available for writing");
    if (m_fileid > 0) {
      sprintf(filename, "%s%02d/storage/%s.%4.4d.%6.6d.sroot-%d",
              dir.c_str(), m_diskid, m_runtype.c_str(), expno, runno, m_fileid);
    } else if (m_fileid == 0) {
      sprintf(filename, "%s%02d/storage/%s.%4.4d.%6.6d.sroot",
              dir.c_str(), m_diskid, m_runtype.c_str(), expno, runno);
    }
    m_file = ::open(filename,  O_WRONLY | O_CREAT | O_EXCL, 0664);
    m_path = filename;
    if (m_file < 0) {
      B2ERROR("Failed to open file : " << filename);
      exit(1);
    } else {
      const char* belle2lib = getenv("BELLE2_LOCAL_DIR");
      int rev = atoi(StringUtil::split(popen(StringUtil::form("svn info %s | grep Revision", belle2lib)), ':')[1].c_str());
      int rev_daq = atoi(StringUtil::split(popen(StringUtil::form("svn info %s/daq | grep Revision", belle2lib)), ':')[1].c_str());
      int rev_rawdata = atoi(StringUtil::split(popen(StringUtil::form("svn info %s/rawdata | grep Revision", belle2lib)),
                                               ':')[1].c_str());
      int rev_svd = atoi(StringUtil::split(popen(StringUtil::form("svn info %s/svd | grep Revision", belle2lib)), ':')[1].c_str());
      int rev_pxd = atoi(StringUtil::split(popen(StringUtil::form("svn info %s/pxd | grep Revision", belle2lib)), ':')[1].c_str());
      char sql[1000];
      sprintf(sql,
              "insert into %s (path_sroot, runtype, expno, runno, fileid, diskid, rev, rev_daq, rev_rawdata, rev_svd, rev_pxd, time_create)"
              " values ('%s:%s', '%s', %d, %d, %d, %d,  %d, %d, %d, %d, %d, current_timestamp) returning id;",
              g_table, m_host.c_str(), filename, m_runtype.c_str(), expno, runno, m_fileid, m_diskid, rev, rev_daq, rev_rawdata, rev_svd,
              rev_pxd);
      try {
        m_db.connect();
        m_db.execute(sql);
        DBRecordList record_v(m_db.loadRecords());
        m_id = (record_v.size() > 0) ? record_v[0].getInt("id") : 0;
        m_db.close();
      } catch (const DBHandlerException& e) {
        LogFile::error("Failed to access db for read: %s", e.what());
        std::ofstream fout(m_dbtmp.c_str(), std::ios::app);
        fout << sql << std::endl;
        fout.close();
        return -1;
      }
    }
    B2INFO("New file " << filename << " is opened");
    return m_id;
  }

  void close()
  {
    if (m_file > 0) {
      ::close(m_file);
      char sql[1000];
      sprintf(sql, "update %s set time_close = current_timestamp, "
              "filesize = %u where id = %d", g_table, m_filesize, m_id);
      try {
        m_db.connect();
        m_db.execute(sql);
        m_db.close();
      } catch (const DBHandlerException& e) {
        B2ERROR("Failed to access db for update: " << e.what());
        std::ofstream fout(m_dbtmp.c_str(), std::ios::app);
        fout << sql << std::endl;
        fout.close();
      }
    }
  }

  int write(char* evbuf, int nbyte)
  {
    int ret = ::write(m_file, evbuf, nbyte);
    m_filesize += nbyte;
    return ret;
  }

  operator bool()
  {
    return m_file > 0;
  }

private:
  DBInterface& m_db;
  std::string m_runtype;
  std::string m_host;
  std::string m_dbtmp;
  int m_id;
  std::string m_path;
  int m_diskid;
  int m_fileid;
  int m_file;
  unsigned int m_filesize;

};

FileHandler* g_file = NULL;

void signalHandler(int)
{
  if (g_file) g_file->close();
  exit(1);
}

int main(int argc, char** argv)
{
  if (argc < 8) {
    printf("%s : <ibufname> <ibufsize> <hostname> <runtype> <path> <ndisk> "
           "<filepath_dbtmp> [ <obufname> <obufsize> nodename, nodeid]\n", argv[0]);
    return 1;
  }
  const unsigned interval = 1;
  const char* ibufname = argv[1];
  const int ibufsize = atoi(argv[2]);
  const char* hostname = argv[3];
  const char* runtype = argv[4];
  const char* path = argv[5];
  const int ndisks = atoi(argv[6]);
  const char* file_dbtmp = argv[7];
  const char* obufname = (argc > 7) ? argv[8] : "";
  const int obufsize = (argc > 8) ? atoi(argv[9]) : -1;
  const char* nodename = (argc > 9) ? argv[10] : "";
  const int nodeid = (argc > 10) ? atoi(argv[11]) : -1;
  RunInfoBuffer info;
  const bool use_info = nodeid >= 0;
  if (use_info) info.open(nodename, nodeid);
  SharedEventBuffer ibuf;
  ibuf.open(ibufname, ibufsize * 1000000);//, true);
  signal(SIGINT, signalHandler);
  signal(SIGKILL, signalHandler);
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  SharedEventBuffer obuf;
  if (obufsize > 0) obuf.open(obufname, obufsize * 1000000);//, true);
  if (use_info) info.reportReady();
  B2DEBUG(1, "started recording.");
  int* evtbuf = new int[10000000];
  unsigned long long nbyte_out = 0;
  unsigned int count_out = 0;
  unsigned int expno = 0;
  unsigned int runno = 0;
  unsigned int subno = 0;
  g_file = new FileHandler(db, runtype, hostname, file_dbtmp);
  FileHandler& file(*g_file);
  SharedEventBuffer::Header iheader;
  int ecount = 0;
  bool newrun = false;
  while (true) {
    if (use_info) info.reportRunning();
    ibuf.read(evtbuf, true, &iheader);
    int nbyte = evtbuf[0];
    int nword = (nbyte - 1) / 4 + 1;
    bool isnew = false;
    if (!newrun || expno < iheader.expno || runno < iheader.runno) {
      newrun = true;
      isnew = true;
      unsigned int expno_tmp = expno;
      unsigned int runno_tmp = runno;
      expno = iheader.expno;
      runno = iheader.runno;
      if (use_info) {
        info.setExpNumber(expno);
        info.setRunNumber(runno);
        info.setSubNumber(subno);
        info.setInputCount(0);
        info.setInputNBytes(0);
        info.setOutputCount(0);
        info.setOutputNBytes(0);
        nbyte_out = 0;
        count_out = 0;
      }
      obuf.lock();
      SharedEventBuffer::Header* oheader = ibuf.getHeader();
      oheader->expno = expno;
      oheader->runno = runno;
      obuf.unlock();
      if (file) file.close();
      file.open(path, ndisks, expno, runno);
      if (expno_tmp > 0 || runno_tmp > 0) {
        std::stringstream ss;
        ss << "update " << g_table << " set time_runend = current_timestamp where expno = " << expno_tmp << " and runno = " << runno_tmp <<
           " and time_runend is null;";
        try {
          db.connect();
          db.execute(ss.str().c_str());
          db.close();
        } catch (const DBHandlerException& e) {
          LogFile::error("Failed to update : %s", e.what());
          return 1;
        }
        //std::string cmd = StringUtil::form("submit_filejob -e %d -r %d", expno_tmp, runno_tmp);
        std::string cmd = StringUtil::form("submit_filejob -all");
        system(cmd.c_str());
        std::cout << "[INFO] " << cmd << std::endl;;
      }
    }
    if (use_info) {
      info.addInputCount(1);
      info.addInputNBytes(nbyte);
    }
    if (file) {
      if (nbyte_out > MAX_FILE_SIZE) {
        file.close();
        nbyte_out = 0;
        file.open(path, ndisks, expno, runno);
      }
      file.write((char*)evtbuf, nbyte);
      nbyte_out += nbyte;
      if (!isnew && obufsize > 0 && count_out % interval == 0 && obuf.isWritable(nword)) {
        obuf.write(evtbuf, nword, true);
      }
      count_out++;
      if (use_info) {
        info.addOutputCount(1);
        info.addOutputNBytes(nbyte);
        info.get()->reserved[0] = file.getFileId();
        info.get()->reserved[1] = file.getDiskId();
        info.get()->reserved_f[0] = (float)info.getOutputNBytes() / 1024. / 1024.;
      }
    } else {
      if (!ecount) {
        B2WARNING("no run was initialzed for recording : " << iheader.expno << "." << iheader.runno);
      }
      ecount = 1;
    }
  }
  return 0;
}

