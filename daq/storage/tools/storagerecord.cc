//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <unistd.h>
#include <cstdlib>

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
const unsigned long long MAX_FILE_SIZE = 4 * GB;
int g_nfiles = 0;
int g_nfiles_closed = 0;
int g_diskid = 0;
std::string g_file_diskid;
std::string g_file_nfiles;
std::string g_file_dbtmp;
Mutex g_mutex;
DBInterface* g_db = NULL;

bool insertdb(const std::string& sql)
{
  try {
    g_db->connect();
    g_db->execute(sql.c_str());
    g_db->close();
  } catch (const DBHandlerException& e) {
    std::ofstream fout(g_file_dbtmp.c_str(), std::ios::app);
    fout << sql << std::endl;
    fout.close();
    return false;
  }
  return true;
}

class FileHandler {

private:
  FILE* m_file;
  char* m_buf;
  int m_id;
  std::string m_path;

public:
  FileHandler() {
    m_file = NULL;
    m_buf = NULL;
    m_id = 0;
  }
  FileHandler(const std::string& dir, int ndisks, int expno, int runno) {
    open(dir, ndisks, expno, runno);
  }
  ~FileHandler() throw() {}

public:
  bool open(const std::string& dir, int ndisks, int expno, int runno) {
    char filename[1024];
    g_mutex.lock();
    bool available = true;
    if (ndisks > 0) {
      int diskid = g_diskid;
      if (g_diskid == 0) {
        std::ifstream fin(g_file_diskid.c_str());
        fin >> g_diskid;
        if (g_diskid == 0) g_diskid = 1;
      }
      available = false;
      for (int i = 0; i < ndisks; i++) {
        struct statvfs statfs;
        sprintf(filename, "%s%02d", dir.c_str(), g_diskid);
        statvfs(filename, &statfs);
        float usage = 1 - ((float)statfs.f_bfree / statfs.f_blocks);
        if (usage < 0.9) {
          sprintf(filename, "%s%02d/storage/full_flag", dir.c_str(), g_diskid);
          std::ifstream fin(filename);
          int flag = 0;
          fin >> flag;
          if (flag != 1) {
            available = true;
            break;
          }
        }
        g_diskid++;
        if (g_diskid > ndisks) g_diskid = 1;
      }
      if (!available) {
        B2FATAL("No disk available for writing");
      }

      if (diskid != g_diskid) {
        std::ofstream fout(g_file_diskid.c_str());
        fout << g_diskid;
        fout.close();
        sprintf(filename, "%s%02d/storage/full_flag", dir.c_str(), diskid);
        fout.open(filename);
        fout << 1;
      }
      if (g_nfiles > 0) {
        sprintf(filename, "%s%02d/storage/e%4.4dr%6.6d.sroot-%d",
                dir.c_str(), g_diskid, expno, runno, g_nfiles);
      } else {
        sprintf(filename, "%s%02d/storage/e%4.4dr%6.6d.sroot",
                dir.c_str(), g_diskid, expno, runno);
      }
    } else {
      if (g_nfiles > 0) {
        sprintf(filename, "%s/storage/e%4.4dr%6.6d.sroot-%d",
                dir.c_str(), expno, runno, g_nfiles);
      } else {
        sprintf(filename, "%s/storage/e%4.4dr%6.6d.sroot",
                dir.c_str(), expno, runno);
      }
    }
    g_nfiles++;
    std::ofstream fout(g_file_nfiles.c_str());
    fout << g_nfiles << " " << expno << " " << runno;
    m_id = g_nfiles;
    g_mutex.unlock();
    m_file = fopen(filename, "w");
    m_path = filename;
    if (m_file == NULL) {
      B2ERROR("failed to open file : " << filename);
      return false;
    } else {
      insertdb(StringUtil::form("insert into fileinfo (path, expno, runno, fileid, diskid, time_create)"
                                " values ('%s', %d, %d, %d, %d, current_timestamp);",
                                filename, expno, runno, m_id - 1, g_diskid));
      m_buf = (char*)malloc(MAX_FILE_SIZE / 100);
      setvbuf(m_file, m_buf, _IOFBF, MAX_FILE_SIZE / 100);
      B2INFO("file " << filename << " opened");
    }
    return available;
  }

  void close() {
    if (m_file != NULL) {
      fclose(m_file);
      insertdb(StringUtil::form("update fileinfo set time_close = "
                                "current_timestamp where path = '%s'",
                                m_path.c_str()));
      g_nfiles_closed++;
    }
    if (m_buf != NULL) {
      free(m_buf);
    }
    m_file = NULL;
    m_buf = NULL;
  }

  int write(char* evbuf, int nbyte) {
    return fwrite(evbuf, nbyte, 1, m_file);
  }

  operator bool() {
    return m_file != NULL;
  }

};

class FileCloser {

public:
  static FileHandler getFile() {
    g_mutex.lock();
    while (g_file_q.empty()) {
      g_cond.wait(g_mutex);
    }
    FileHandler file = g_file_q.front();
    g_file_q.pop();
    g_mutex.unlock();
    return file;
  }

public:
  static void closeAll() {
    while (!g_file_q.empty()) {
      g_file_q.front().close();
      g_file_q.pop();
    }
  }

private:
  static std::queue<FileHandler> g_file_q;
  static Mutex g_mutex;
  static Cond g_cond;

private:
  FileHandler m_handler;
  std::string m_dir;
  int m_ndisks;
  int m_expno;
  int m_runno;

public:
  FileCloser(const FileHandler& handler,
             const std::string dir,
             int ndisks, int expno, int runno)
    : m_handler(handler), m_dir(dir),
      m_ndisks(ndisks), m_expno(expno), m_runno(runno) {}

public:
  void run() {
    g_mutex.lock();
    g_file_q.push(FileHandler(m_dir, m_ndisks, m_expno, m_runno));
    g_cond.signal();
    g_mutex.unlock();
    m_handler.close();
  }

};

std::queue<FileHandler> FileCloser::g_file_q;
Mutex FileCloser::g_mutex;
Cond FileCloser::g_cond;

void signalHandler(int)
{
  FileCloser::closeAll();
  exit(1);
}

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("rawfile2rb : ibufname ibufsize path "
           "filepath_diskid filepath_nfile filepath_dbtmp "
           "ndisk obufname obufsize [nodename, nodeid]\n");
    return 1;
  }
  signal(SIGINT, signalHandler);
  ConfigFile config("slowcontrol");
  g_db = new PostgreSQLInterface(config.get("database.host"),
                                 config.get("database.dbname"),
                                 config.get("database.user"),
                                 config.get("database.password"),
                                 config.getInt("database.port"));
  const unsigned interval = 10;
  RunInfoBuffer info;
  const bool use_info = (argc > 10);
  if (use_info) {
    info.open(argv[9], atoi(argv[10]));
  }
  SharedEventBuffer ibuf;
  ibuf.open(argv[1], atol(argv[2]) * 1000000, true);
  const std::string dir = argv[3];
  int ndisks = atoi(argv[4]);
  g_file_diskid = argv[5];
  g_file_nfiles = argv[6];
  g_file_dbtmp = argv[7];
  unsigned int expno_tmp = 0;
  unsigned int runno_tmp = 0;
  std::ifstream fin(g_file_nfiles.c_str());
  fin >> g_nfiles >> expno_tmp >> runno_tmp;
  SharedEventBuffer obuf;
  obuf.open(argv[8], atol(argv[9]) * 1000000, true);
  B2INFO("storagerecord: started recording.");
  if (use_info) {
    info.reportRunning();
  }

  int* evtbuf = new int[10000000];
  unsigned long long nbyte_out = 0;
  unsigned int count_out = 0;
  unsigned int expno = 0;
  unsigned int runno = 0;
  unsigned int subno = 0;
  FileHandler file;
  SharedEventBuffer::Header iheader;
  while (true) {
    ibuf.read(evtbuf, true, &iheader);
    int nbyte = evtbuf[0];
    int nword = (nbyte - 1) / 4 + 1;
    if (expno < iheader.expno || runno < iheader.runno) {
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
      if (expno_tmp != expno || runno_tmp != runno) {
        g_nfiles = 0;
        expno_tmp = 0;
        runno_tmp = 0;
      }
      std::ofstream fout(g_file_nfiles.c_str());
      fout << g_nfiles;
      file.open(dir, ndisks, expno, runno);
      PThread(new FileCloser(FileHandler(), dir, ndisks, expno, runno));
    }
    if (use_info) {
      info.addInputCount(1);
      info.addInputNBytes(nbyte);
    }
    if (file) {
      if (nbyte_out > MAX_FILE_SIZE) {
        PThread(new FileCloser(file, dir, ndisks, expno, runno));
        nbyte_out = 0;
        file = FileCloser::getFile();
      }
      file.write((char*)evtbuf, nbyte);
      nbyte_out += nbyte;
      if (count_out % interval == 0 && obuf.isWritable(nword)) {
        obuf.write(evtbuf, nword, true);
      }
      count_out++;
      if (use_info) {
        info.addOutputCount(1);
        info.addOutputNBytes(nbyte);
        info.get()->reserved[0] = g_nfiles_closed;
        info.get()->reserved[1] = g_diskid;
        info.get()->reserved_f[0] = (float)info.getOutputNBytes() / 1024. / 1024.;
      }
    } else {
      B2ERROR("storagerecord: no run was initialzed for recording");
      return 1;
    }
    //usleep(1000);
  }
  return 0;
}

