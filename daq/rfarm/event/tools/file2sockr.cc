//+
// File : file2rb.cc
// Description : Get an event from a SeqRoot file and place it in Rbuf
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Apr - 2012
//-
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "framework/pcore/SeqFile.h"
#include "daq/dataflow/REvtSocket.h"

#include "TRandom.h"

#define EVENTINTERVAL 5000

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("file2sock : filename port poisson_freq file_interval\n");
    exit(-1);
  }

  // Poisson random number generate
  TRandom rand;

  string filename(argv[1]);
  int port = atoi(argv[2]);
  int pfreq = atoi(argv[3]);

  double minterval = 0.0;
  if (pfreq != 0)
    minterval = 1.0E6 / (double)pfreq;

  vector<string> filelist;
  // Check file
  if ((int)filename.rfind(".list") != -1) {
    FILE* fd = fopen(filename.c_str(), "r");
    for (;;) {
      char listfile[1024];
      int is = fscanf(fd, "%s", listfile);
      if (is <= 0) break;
      filelist.push_back(string(listfile));
    }
  } else {
    filelist.push_back(filename);
    printf("File %s is put ln the list\n", filename.c_str());
  }
  printf("# of input files = %d\n", (int)filelist.size());
  for (int i = 0; i < (int)filelist.size(); i++) {
    printf("file = %s\n", filelist[i].c_str());
  }

  int fileptr = 0;

  // Open EventSocket
  REvtSocketSend* sock = new REvtSocketSend(port);

  // Event Buffer
  char* evbuf = new char[MAXEVTSIZE];

  // Open 1st file
  SeqFile* file = new SeqFile(filelist[fileptr++].c_str(), "r");
  if (file->status() <= 0) {
    perror("file open");
    exit(-1);
  }
  // Skip the first record (StreamerInfo)
  int is = file->read(evbuf, MAXEVTSIZE);
  if (is <= 0) {
    printf("Error in reading file : %d\n", is);
    exit(-1);
  }

  // Event / time counter
  int nevent = 0;
  struct timeval tnow;
  struct timeval tprev;
  gettimeofday(&tnow, NULL);
  gettimeofday(&tprev, NULL);

  double datasize = 0.0;
  printf("Start event loop\n");
  // Loop for event records
  for (;;) {
    int is = file->read(evbuf, MAXEVTSIZE);
    if (is < 0) {
      printf("Error in reading file : %d\n", is);
      break;
    } else if (is > MAXEVTSIZE) {
      printf("Event size too large : %d\n", is);
      continue;
    } else if (is == 0) {
      delete file;
      if (fileptr == (int)filelist.size()) {
        printf("End of file list reached. Exitting\n");
        break;
      }
      file = new SeqFile(filelist[fileptr++].c_str(), "r");
      if (file->status() <= 0) {
        perror("file open");
        exit(-1);
      }
      // Skip the first record (StreamerInfo)
      int is = file->read(evbuf, MAXEVTSIZE);
      if (is <= 0) {
        printf("Error in reading file : %d\n", is);
        exit(-1);
      }
      // Read next record (Event)
      is = file->read(evbuf, MAXEVTSIZE);
      if (is < 0) {
        printf("Error in reading file : %d\n", is);
        exit(-1);
      }
      // Wait for 5 sec so that processing of previous file is completed.
      sleep(10);
    }

    // Put the message to Socket
    EvtMessage* msg = new EvtMessage(evbuf);    // Ptr copy, no overhead

    if (msg->type() == MSG_TERMINATE) {
      printf("EoF found. Exitting.....\n");
      sock->send(msg);
      delete msg;
      return -1;
    } else if (msg->type() == MSG_STREAMERINFO) {
      printf("StreamerInfo. Skipped....\n");
      continue;
    } else {
      int is = sock->send(msg);
      delete msg;
      if (is <= 0) {
        printf("Cannot send event. Exitting\n");
        return -1;
      }
      //    return msg->size();
    }
    if (minterval != 0)
      usleep(rand.Poisson(minterval));

    nevent++;
    datasize += (double)is;

    if (nevent % EVENTINTERVAL == 0) {
      gettimeofday(&tnow, NULL);
      double delta = (double)((tnow.tv_sec - tprev.tv_sec) * 1000000 +
                              (tnow.tv_usec - tprev.tv_usec));
      double rate = ((double)EVENTINTERVAL) / delta * 1.0E6;
      double flow = datasize / delta;
      printf("Event = %8d; Ave. rate = %7.2f Hz, flow = %7.2f MB/s\n",
             nevent, rate, flow);
      tprev = tnow;
      datasize = 0.0;
    }
  }
}


