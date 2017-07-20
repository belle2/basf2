//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <fstream>

#include <framework/pcore/EvtMessage.h>
#include <daq/storage/BinData.h>

#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/statvfs.h>
#include <zlib.h>

using namespace Belle2;
using namespace std;

#define MAXBUF  400000000

unsigned int g_nbyte = 0;
unsigned int g_chksum = 1;
unsigned int g_nevents = 0;

int write_chksum(int wfd, int* evtbuf, int nbyte, bool isstreamer = false)
{
  int ret = ::write(wfd, (char*)evtbuf, nbyte);
  g_nbyte += nbyte;
  if (!isstreamer) {
    g_nevents++;
  }
  g_chksum = adler32(g_chksum, (unsigned char*)evtbuf, nbyte);
  return ret;
}

int main(int argc, char** argv)
{
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  db.connect();
  //std::ifstream fin(argv[1]);
  std::string path;
  std::ofstream fout("list_send.retry");
  //while (fin >> path) {
  //db.execute("select * from datafiles where path = '%s' and host = 'HLT1';", path.c_str());
  db.execute("select * from datafiles where expno=1 and runno<= 03519 and runno >= 03490 and fileno = 0 and host = 'HLT1';");
  DBRecordList records(db.loadRecords());
  for (size_t i = 0; i < records.size(); i++) {
    unsigned long long nevents_old = records[i].getInt("nevents");
    std::string path = records[i].get("path");
    int fd = ::open(path.c_str(), O_RDONLY);
    if (fd < 0) {
      perror("open");
      return 1;
    }
    int* sbuf = new int[10000000];
    int* ebuf = new int[10000000];
    unsigned int nbyte = 0;
    EvtMessage* msg = NULL;
    unsigned long long nevents = 0;
    while (true) {
      int sstat = read(fd, sbuf, sizeof(int));
      if (sstat <= 0) {
        break;
      }
      nbyte = sbuf[0];
      int rstat = read(fd, (sbuf + 1), nbyte - sizeof(int));
      if (rstat <= 0) break;
      if (sstat + rstat != sbuf[0]) break;
      msg = new EvtMessage((char*)sbuf);
      int type = msg->type();
      delete msg;
      if (type == MSG_STREAMERINFO) {
        continue;
      } else if (type == MSG_EVENT) {
        nevents++;
      }
    }
    ::close(fd);

    if (nevents != nevents_old) {
      LogFile::info("update datafiles set nevents = %lu "
                    "where path = '%s' and host = 'HLT1';",
                    nevents, path.c_str());
      LogFile::info("nevents >> %lu (%lu) file = '%s';",
                    nevents, nevents_old, path.c_str());
      db.execute("update datafiles set nevents = %lu "
                 "where path = '%s' and host = 'HLT1';",
                 nevents, path.c_str());
    }
  }
  return 0;
}

