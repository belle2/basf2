#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <framework/logging/Logger.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/SeqFile.h>

#include <daq/storage/BinData.h>
#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/database/DBObject.h>
#include <daq/slc/database/DBObjectLoader.h>

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
#include <zlib.h>

using namespace Belle2;

const unsigned long long GB = 1000 * 1024 * 1024;
const unsigned long long MAX_FILE_SIZE = 2 * GB;
const char* g_table = "datafiles";
unsigned int g_streamersize = 0;
char* g_streamerinfo = new char[1000000];
int g_diskid = 0;

class FileHandler {

public:
  FileHandler(DBInterface* db, const std::string& runtype,
              const char* host, const char* dbtmp)
    : m_db(db), m_runtype(runtype), m_host(host), m_dbtmp(dbtmp)
  {
    m_file = -1;
    m_filesize = 0;
    m_fileid = 0;
    m_diskid = 1;
  }
  ~FileHandler() throw()
  {
  }

public:
  int getDiskId() { return m_diskid; }
  int getFileId() { return m_fileid; }

public:
  int open(const std::string& dir, int ndisks, int expno, int runno, int fileid)
  {
    m_filesize = 0;
    m_fileid = 0;
    m_diskid = 1;
    m_chksum = 1;
    m_nevents = 0;
    m_filesize = 0;
    m_expno = expno;
    m_runno = runno;
    m_fileid = fileid;
    bool available = false;
    for (int i = 0; i < ndisks; i++) {
      struct statvfs statfs;
      char filename[1024];
      sprintf(filename, "%s%02d", dir.c_str(), m_diskid);
      statvfs(filename, &statfs);
      float usage = 1 - ((float)statfs.f_bfree / statfs.f_blocks);
      if (usage < 0.9) {
        sprintf(filename, "%s%02d/storage/full_flag", dir.c_str(), m_diskid);
        std::ifstream fin(filename);
        int flag = 0;
        fin >> flag;
        if (flag != 1) {
          available = true;
          std::cout << "[DEBUG] disk : " << m_diskid << " is available" << std::endl;
          break;
        }
        fin.close();
        std::cout << "[DEBUG] disk : " << m_diskid << " is still full" << std::endl;
      } else {
        sprintf(filename, "%s%02d/storage/full_flag", dir.c_str(), m_diskid);
        std::ofstream fout(filename);
        fout << 1;
        fout.close();
        B2WARNING("disk-" << m_diskid << " is full " << usage);
      }
      m_diskid++;
      if (m_diskid > ndisks) m_diskid = 1;
    }
    if (!available) {
      B2FATAL("No disk available for writing");
      exit(1);
    }
    std::string filedir = dir + StringUtil::form("%02d/storage/%4.4d/%5.5d/",
                                                 m_diskid, expno, runno);
    system(("mkdir -p " + filedir).c_str());
    m_filename = StringUtil::form("%s.%4.4d.%5.5d.%s.f%5.5d.sroot",
                                  m_runtype.c_str(), expno, runno, m_host.c_str(), m_fileid);
    if (g_diskid > 0 && g_diskid != m_diskid) {
      B2FATAL("disk-" << m_diskid << " is already full! Terminating process..");
      exit(1);
    }
    g_diskid = m_diskid;
    m_path = filedir + m_filename;
    m_file = ::open(m_path.c_str(),  O_WRONLY | O_CREAT | O_EXCL, 0664);
    if (m_file < 0) {
      B2FATAL("Failed to open file : " << m_path);
      exit(1);
    }
    try {
      m_db->connect();
      m_db->execute("insert into %s (name, path, host, label, expno, runno, fileno, nevents, chksum, size) "
                    "values ('%s', '%s', '%s', '%s', %d, %d, %d, 0, 0, 0);",
                    g_table, m_filename.c_str(), m_path.c_str(), m_host.c_str(),
                    m_runtype.c_str(), m_expno, m_runno, m_fileid);
    } catch (const DBHandlerException& e) {
      B2WARNING(e.what());
    }
    write(g_streamerinfo, g_streamersize, true);
    B2INFO("New file " << m_path << " is opened");

    return m_id;
  }

  void close()
  {
    if (m_file > 0) {
      std::cout << "[DEBUG] File closed" << std::endl;
      ::close(m_file);
      try {
        struct stat st;
        stat(m_path.c_str(), &st);
        std::string d = Date(st.st_mtime).toString();
        //if (m_fileid == 0) m_nevents--;//1 entry for StreamerInfo
        m_db->connect();
        m_db->execute("update %s set time_close = '%s', chksum = %lu, nevents = %lu, "
                      "size = %lu where name = '%s' and host = '%s';",
                      g_table, d.c_str(), m_chksum, m_nevents, m_filesize,
                      m_filename.c_str(), m_host.c_str());
      } catch (const DBHandlerException& e) {
        B2WARNING(e.what());
      }
      m_db->close();
    }
  }

  int write(char* evtbuf, int nbyte, bool isstreamer = false)
  {
    int ret = ::write(m_file, evtbuf, nbyte);
    m_filesize += nbyte;
    if (!isstreamer) {
      m_nevents++;
    }
    m_chksum = adler32(m_chksum, (unsigned char*)evtbuf, nbyte);
    return ret;
  }

  operator bool()
  {
    return m_file > 0;
  }

private:
  DBInterface* m_db;
  std::string m_runtype;
  std::string m_host;
  std::string m_dbtmp;
  int m_id;
  std::string m_filename;
  std::string m_path;
  int m_diskid;
  int m_fileid;
  int m_file;
  std::string m_configname;
  int m_expno;
  int m_runno;
  unsigned long long m_filesize;
  unsigned long long m_chksum;
  unsigned long long m_nevents;
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
           "<filepath_dbtmp> [<obufname> <obufsize> nodename, nodeid]\n", argv[0]);
    return 1;
  }
  //storagerecord STORE02:REC 10 HLT2 debug /rawdata/disk 11 /home/usr/stordaq/data/dbtmp.txt STORE02:OUT 10 store02_storagerecord 3
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
  const unsigned int ninput = (argc > 11) ? atoi(argv[12]) : 1;

  RunInfoBuffer info;
  const bool use_info = nodeid >= 0;
  if (use_info) {
    info.open(nodename, nodeid);
  }
  SharedEventBuffer ibuf[10];
  for (unsigned int ib = 0; ib < ninput; ib++) {
    ibuf[ib].open(StringUtil::form("%s_%d", ibufname, ib), ibufsize * 1000000);//, true);
  }
  signal(SIGINT, signalHandler);
  signal(SIGKILL, signalHandler);
  ConfigFile config("slowcontrol");
  PostgreSQLInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                                    config.get("database.dbname"),
                                                    config.get("database.user"),
                                                    config.get("database.password"),
                                                    config.getInt("database.port"));
  SharedEventBuffer obuf;
  if (obufsize > 0) obuf.open(obufname, obufsize * 1000000);//, true);
  if (use_info) info.reportReady();
  B2DEBUG(1, "started recording.");
  unsigned long long nbyte_out = 0;
  unsigned int count_out = 0;
  unsigned int expno = 0;
  unsigned int runno = 0;
  unsigned int subno = 0;
  int* evtbuf = new int[10000000];
  g_file = new FileHandler(db, runtype, hostname, file_dbtmp);
  FileHandler& file(*g_file);
  int ecount = 0;
  bool newrun = false;
  unsigned int fileid = 0;
  struct dataheader {
    int nword;
    int type;
    unsigned int expno;
    unsigned int runno;
  } hd;
  g_streamersize = 0;
  while (true) {
    if (use_info) info.reportRunning();
    for (unsigned int ib = 0; ib < ninput; ib++) {
      ibuf[ib].lock();
      ibuf[ib].read((int*)&hd, true, true);
      ibuf[ib].read(evtbuf, true, true);
      ibuf[ib].unlock();
      int nbyte = evtbuf[0];
      int nword = (nbyte - 1) / 4 + 1;
      bool isnew = false;
      if (hd.type == MSG_STREAMERINFO) {
        memcpy(g_streamerinfo, evtbuf, nbyte);
        g_streamersize = nbyte;
      }
      if (expno > hd.expno || runno > hd.runno) {
        B2WARNING("Old run was detected => discard event exp = " << hd.expno << " (" << expno << "), runno" << hd.runno << "(" << runno <<
                  ")");
        continue;
      }
      if (!newrun || expno < hd.expno || runno < hd.runno) {
        newrun = true;
        isnew = true;
        expno = hd.expno;
        runno = hd.runno;
        fileid = 0;
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
        SharedEventBuffer::Header* oheader = obuf.getHeader();
        oheader->expno = expno;
        oheader->runno = runno;
        obuf.unlock();
        if (file) {
          file.close();
        }
        file.open(path, ndisks, expno, runno, fileid);
        nbyte_out += nbyte;
        fileid++;
        continue;
      }
      if (use_info) {
        info.addInputCount(1);
        info.addInputNBytes(nbyte);
      }
      if (hd.type == MSG_STREAMERINFO) {
        continue;
      }
      if (file) {
        if (nbyte_out > MAX_FILE_SIZE) {
          file.close();
          nbyte_out = 0;
          file.open(path, ndisks, expno, runno, fileid);
          fileid++;
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
          B2WARNING("no run was initialzed for recording : " << hd.expno << "." << hd.runno);
        }
        ecount = 1;
      }
    }
  }
  return 0;
}

