#include <daq/storage/BinData.h>
#include <daq/storage/SharedEventBuffer.h>
#include <daq/slc/system/Time.h>

#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>

#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/statvfs.h>
#include <unistd.h>

using namespace Belle2;

const unsigned int NWORD_BUFFER = 2000000;//20kB
const unsigned int interval = 10000;

int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("%s : filename1 bufsize bufname\n", argv[0]);
    return 1;
  }
  const char* filename = argv[1];
  FILE* file = NULL;
  if (strcmp(filename, "/dev/null") != 0) {
    file = fopen(filename, "w");
  }
  unsigned long long bufsize = atol(argv[2]) * 1000000;

  SharedEventBuffer sbuf;
  const char* nodename = argv[3];
  sbuf.open(nodename, bufsize);
  unsigned int nword = 0;
  int* evtbuf = new int[NWORD_BUFFER];
  int* compbuf = new int[NWORD_BUFFER];
  Time t0;
  double datasize = 0;
  BinData data;
  unsigned int count = 0;
  while (true) {
    sbuf.read(evtbuf, false, false);
    nword = evtbuf[0];
    if (file) {
      fwrite(evtbuf, nword * sizeof(int), 1, file);
    }
    count++;
    datasize += nword * sizeof(int);
    if (count % interval == 0) {
      Time t;
      double dt = (t.get() - t0.get());
      double freq = interval / dt  / 1000.;
      double rate = datasize / dt / 1000. / 1000. / 1000. * 8;
      fprintf(stderr, "Serial = %u Freq = %f [kHz], Rate = %f [Gbps], DataSize = %f [kB/event]\n",
              count, freq, rate, datasize / 1000. / interval);
      t0 = t;
      datasize = 0;
    }
  }
  return 0;
}
