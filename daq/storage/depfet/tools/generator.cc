#include <daq/storage/BinData.h>
#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>

#include <fstream>
#include <vector>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include <zlib.h>

using namespace std;
using namespace Belle2;

unsigned int expno = 0;
unsigned int runno = 0;
unsigned int subno = 0;
unsigned int evtno = 1;
unsigned int nword;
int nout;
SharedEventBuffer obuf[10];
RunInfoBuffer info;

void help(int val, char** argv);
void set_arguments(int argc, char** argv);

int main(int argc, char** argv)
{
  if (argc < 4) help(1, argv);
  set_arguments(argc, argv);
  srand((unsigned) time(NULL));
  const unsigned int id = atoi(argv[1]);
  int* evtbuf = new int[1000000];
  BinData data;
  data.setBuffer(evtbuf);
  const int interval = 10000;
  Time t0;
  double datasize = 0;
  unsigned int count = 0;
  for (unsigned int i = 0; i < nword; i++) {
    evtbuf[i + 6] = rand() % 256;
  }
  if (info.isAvailable()) {
    info.setExpNumber(expno);
    info.setRunNumber(runno);
    info.setInputCount(0);
    info.setInputNBytes(0);
    info.setOutputCount(0);
    info.setOutputNBytes(0);
    info.reportReady();
  }
  info.reportReady();
  while (true) {
    data.setExpNumber(expno);
    data.setRunNumber(runno, 0);
    data.setEventNumber(evtno);
    data.setNBoard(1);
    data.setNodeId(id);
    data.setBodyWordSize(nword);
    obuf[evtno % nout].write(data.getBuffer(), data.getWordSize(), true);
    evtno++;
    count++;
    datasize += data.getByteSize();
    if (count % 10 == 0) usleep(1);
    if (count % interval == 0) {
      Time t;
      double dt = (t.get() - t0.get());
      double freq = interval / dt  / 1000.;
      double rate = datasize / dt / 1000. / 1000. / 1000. * 8;
      fprintf(stderr,
              "Serial = %u Freq = %f [kHz], Rate = %f [Gbps], DataSize = %f [kB/event]\n",
              count, freq, rate, datasize / 1000. / interval);
      t0 = t;
      datasize = 0;
    }
    if (info.isAvailable()) {
      info.addInputCount(1);
      info.addInputNBytes(data.getByteSize());
      info.addOutputCount(1);
      info.addOutputNBytes(data.getByteSize());
    }
  }
  return 0;
}

void help(int val, char** argv)
{
  printf("%s : id -e expno -r runno -w nword "
         "-o [bufname1,bufname2,...]:size "
         "-m memname:size\n", argv[0]);
  exit(val);
}

void set_arguments(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "o:m:e:r:w:h::")) != -1) {
    switch (opt) {
      case 'o': {
        StringList s = StringUtil::split(optarg, ':');
        if (s.size() > 1) {
          int bsize_org = atoi(s[1].c_str());
          unsigned int bsize = bsize_org * 1000000;
          StringList s1 = StringUtil::split(s[0], ',');
          nout = s1.size();
          for (int i = 0; i < nout; i++) {
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
      case 'e':
        expno = atoi(optarg);
        break;
      case 'r':
        runno = atoi(optarg);
        break;
      case 'w':
        nword = atoi(optarg) * 1024;
        break;
      case 'h':
        help(0, argv);
      default:
        printf("error! \'%c\' \'%c\'\n", opt, optopt);
        help(1, argv);
    }
  }
}
