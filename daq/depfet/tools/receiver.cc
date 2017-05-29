#include <daq/storage/BinData.h>
#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>

#include <unistd.h>

const unsigned int NWORD_BUFFER = 40 * 1024 * 1024;//100kB

using namespace Belle2;

std::vector<SharedEventBuffer> obuf;
int noutput;
TCPSocket socket;
RunInfoBuffer info;

void help(int val, char** argv);
void set_arguments(int argc, char** argv);

int main(int argc, char** argv)
{
  if (argc < 4) help(1, argv);
  set_arguments(argc, argv);
  int* evtbuf = new int[NWORD_BUFFER];
  BinData data(evtbuf);
  int ntried = 0;
  while (true) {
    while (socket.get_fd() <= 0) {
      try {
        fprintf(stderr, "[INFO] Connecting to %s:%d...\n",
                socket.getIP().c_str(), socket.getPort());
        socket.connect();
        //socket.setRecvBufferSize(320 * 1024 * 1024);
        if (info.isAvailable()) {
          info.reportReady();
        }
        fprintf(stderr, "[INFO] Connected\n");
        ntried = 0;
        break;
      } catch (const IOException& e) {
        socket.close();
        printf("Failed to connect (try = %d)\n", ntried++);
        sleep(5);
      }
    }
    try {
      unsigned int expno = 0;
      unsigned int runno = 0;
      unsigned int subno = 0;
      unsigned int evtno = 0;
      unsigned int count = 0;
      while (true) {
        socket.read(data.getBuffer(), sizeof(int));
        unsigned int nbyte = data.getByteSize() - sizeof(int);
        socket.read(data.getBuffer() + 1, nbyte);
        nbyte += sizeof(int);
        if (info.isAvailable()) {
          info.addInputCount(1);
          info.addInputNBytes(nbyte);
        }
        unsigned int nword = nbyte / 4;
        evtno = data.getEventNumber();
        expno = data.getExpNumber();
        runno = data.getRunNumber();
        int id = ((evtno + 1) / 2) % noutput;
        obuf[id].write(evtbuf, nword, true);
        SharedEventBuffer::Header* header = obuf[id].getHeader();
        header->expno = expno;
        header->runno = runno;
        header->subno = subno;
        if (info.isAvailable()) {
          info.setExpNumber(expno);
          info.setRunNumber(runno);
          info.setSubNumber(subno);
          info.setInputCount(0);
          info.setInputNBytes(0);
          info.setOutputCount(0);
          info.setOutputNBytes(0);
        }
        if (count < 10) {
          LogFile::debug("%d-th event to buf[%d]", evtno, id);
        }
        count++;
        if (info.isAvailable()) {
          info.addOutputCount(1);
          info.addOutputNBytes(nword * sizeof(int));
        }
      }
    } catch (const IOException& e) {
      socket.close();
      printf("Connection broken.\n");
      sleep(5);
    }
  }
  return 0;
}


void help(int val, char** argv)
{
  printf("%s : -i hostname:port "
         "-o [bufname1,bufname2,...]:size "
         "-m memname:size\n", argv[0]);
  exit(val);
}

void set_arguments(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "i:o:m:r:w::")) != -1) {
    switch (opt) {
      case 'i' : {
        StringList s = StringUtil::split(optarg, ':');
        if (s.size() > 1) {
          int port = atoi(s[1].c_str());
          socket.setIP(s[0]);
          socket.setPort(port);
          fprintf(stderr, "%s:%d\n", socket.getIP().c_str(), port);
        } else {
          help(1, argv);
        }
      } break;
      case 'o': {
        StringList s = StringUtil::split(optarg, ':');
        if (s.size() > 1) {
          int bsize_org = atoi(s[1].c_str());
          unsigned int bsize = bsize_org * 1000000;
          StringList s1 = StringUtil::split(s[0], ',');
          noutput = s1.size();
          obuf.resize(noutput);
          for (int i = 0; i < noutput; i++) {
            obuf[i].open(s1[i], bsize);
            fprintf(stderr, "%s:%d\n", s1[i].c_str(), bsize_org);
          }
        } else {
          help(1, argv);
        }
      } break;
      case 'm': {
        StringList s = StringUtil::split(optarg, ':');
        if (s.size() > 1) {
          info.open(s[0], atoi(s[1].c_str()));
        }
      } break;
      case 'h':
        help(0, argv);
      default:
        printf("error! \'%c\' \'%c\'\n", opt, optopt);
        help(1, argv);
    }
  }
}
