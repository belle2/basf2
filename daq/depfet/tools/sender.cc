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

using namespace std;
using namespace Belle2;

void help(int val, char** argv);
void set_arguments(int argc, char** argv);

std::vector<TCPServerSocket> server_socket;
std::vector<TCPSocket> socket;
SharedEventBuffer ibuf;
RunInfoBuffer info;

int main(int argc, char** argv)
{
  if (argc < 4) help(4, argv);
  srand((unsigned) time(NULL));
  set_arguments(argc, argv);
  int* evtbuf = new int[1000000];
  const int interval = 100000;
  while (true) {
    info.reportReady();
    for (size_t i = 0; i < server_socket.size(); i++) {
      fprintf(stderr, "wait for connection[%d] to %s:%d\n", (int)i,
              server_socket[i].getIP().c_str(), server_socket[i].getPort());
      socket[i] = server_socket[i].accept();
      fprintf(stderr, "[INFO] Connected [%d] from %s:%d...\n", (int)i,
              socket[i].getIP().c_str(), socket[i].getPort());
    }
    info.reportRunning();
    try {
      fprintf(stderr, "connected:\n");
      //socket.setSendBufferSize(32 * 1024 * 1024);
      Time t0;
      double datasize = 0;
      unsigned long count = 0;
      int nsock = socket.size();
      while (true) {
        unsigned int nword = 0;
        ibuf.read(evtbuf, false, false);
        nword = evtbuf[0];
        unsigned int evtno = evtbuf[4];
        if (info.isAvailable()) {
          info.addInputCount(1);
          info.addInputNBytes(nword * sizeof(int));
          info.addOutputCount(1);
          info.addOutputNBytes(nword * sizeof(int));
        }
        if (count < 10) {
          fprintf(stderr, "%d-th event\n", count);
        }
        socket[evtno % nsock].write(evtbuf, nword * sizeof(int));
        count++;
        datasize += nword * sizeof(int);
        if (count % interval == 0) {
          Time t;
          double dt = (t.get() - t0.get());
          double freq = interval / dt  / 1000.;
          double rate = datasize / dt / 1000. / 1000. / 1000. * 8;
          fprintf(stderr,
                  "Serial = %lu Freq = %f [kHz], Rate = %f [GBps], DataSize = %f [kB/event]\n",
                  count, freq, rate, datasize / 1000. / interval);
          t0 = t;
          datasize = 0;
        }
      }
    } catch (const IOException& e) {
      return -1;
    }
  }
  return 0;
}

void help(int val, char** argv)
{
  printf("%s : -i bufname:size -o hostname:port -m memname\n", argv[0]);
  exit(val);
}

void set_arguments(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "i:o:m:k:")) != -1) {
    switch (opt) {
      case 'i': {
        StringList s = StringUtil::split(optarg, ':');
        if (s.size() > 1) {
          int bsize_org = atoi(s[1].c_str());
          unsigned int bsize = bsize_org * 1000000;
          ibuf.open(s[0], bsize, true);
        } else {
          help(1, argv);
        }
      } break;
      case 'o': {
        StringList s = StringUtil::split(optarg, ',');
        int nsock = s.size();
        server_socket.resize(nsock);
        socket.resize(nsock);
        for (size_t i = 0; i < s.size(); i++) {
          StringList s1 = StringUtil::split(s[i], ':');
          if (s1.size() > 1) {
            server_socket[i].open(s1[0], atoi(s1[1].c_str()));
          } else {
            help(1, argv);
          }
        }
      } break;
      case 'm': {
        printf("%s\n", optarg);
        StringList s = StringUtil::split(optarg, ':');
        if (s.size() > 1) {
          //info.open(s[0], atoi(s[1].c_str()));
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


