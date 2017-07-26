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
  int fd = ::open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }
  int* sbuf = new int[10000000];
  int* ebuf = new int[10000000];
  unsigned int nbyte = 0;
  EvtMessage* msg = NULL;
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
      break;
    }
  }
  ::close(fd);

  fd = ::open(argv[2], O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }
  int wfd = ::open(argv[3], O_RDWR | O_CREAT, 0666);
  if (wfd < 0) {
    perror("open");
    return 1;
  }
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  write_chksum(wfd, sbuf, nbyte, true);
  msg = NULL;
  int count = 0;
  bool hasstreamer = false;
  while (true) {
    int sstat = read(fd, ebuf, sizeof(int));
    if (sstat <= 0) {
      break;
    }
    nbyte = ebuf[0];
    int rstat = read(fd, (ebuf + 1), nbyte - sizeof(int));
    if (rstat <= 0) break;
    if (sstat + rstat != ebuf[0]) break;
    count++;
    if (count == 1) continue;
    msg = new EvtMessage((char*)ebuf);
    int type = msg->type();
    delete msg;
    if (type != MSG_EVENT) {
      hasstreamer = true;
      continue;
    }
    if (!hasstreamer && (strcmp(argv[1], argv[2]) == 0)) {
      continue;
    }
    write_chksum(wfd, ebuf, nbyte, false);
  }
  db.connect();
  db.execute("update datafiles set chksum = %lu, nevents = %lu, "
             "size = %lu where path = '%s' and host = 'HLT1';",
             g_chksum, g_nevents, g_nbyte, argv[3]);
  return 0;
}

