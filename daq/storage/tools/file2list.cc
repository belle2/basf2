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
#include <sstream>

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
  db.execute("select * from datafiles where expno=1 and runno>= 03519 and runno >= 03490 and fileno = 0 and host = 'HLT1';");
  DBRecordList records(db.loadRecords());
  for (size_t i = 0; i < records.size(); i++) {
    std::string name = records[i].get("name");
    int expno = records[i].getInt("expno");
    int runno = records[i].getInt("runno");
    std::string label = records[i].get("label");
    if (label == "debug") continue;
    std::string path = records[i].get("path");
    std::string host = records[i].get("host");
    int fileno = records[i].getInt("fileno");
    unsigned long long size = records[i].getInt("size");
    unsigned long long nevents = records[i].getInt("nevents");
    unsigned long long chksum = records[i].getInt("chksum");
    std::string s_chksum = StringUtil::form("%x", chksum);
    std::stringstream ss;
    if (chksum > 0) {
      ss << StringUtil::form("%4.4d/%5.5d/", expno, runno) << name << "," << expno << "," << runno << "," << fileno << ","
         << size << "," << nevents << "," << s_chksum << "";
      LogFile::info(ss.str());
      fout << ss.str() << std::endl;
    }
  }
  //}
  fout.close();
  return 0;
}

