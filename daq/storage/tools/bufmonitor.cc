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

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/LogFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("%s : bufname bufsize\n", argv[0]);
    return 1;
  }
  const unsigned interval = 10;
  RunInfoBuffer info;
  SharedEventBuffer ibuf;
  ibuf.open(argv[1], atoi(argv[2]) * 1000000);//, true);
  SharedEventBuffer::Header* hd = ibuf.getHeader();
  while (true) {
    LogFile::debug("%d %d %d %d %ld",
                   hd->expno, hd->runno, hd->subno,
                   hd->count_in - hd->count_out,
                   hd->nword_in - hd->nword_out);
    sleep(1);
  }
  return 0;
}

