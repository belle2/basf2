//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <unistd.h>
#include <cstdlib>
#include <cstring>

#include <framework/logging/Logger.h>

#include <daq/storage/BinData.h>
#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/base/Debugger.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("rawfile2rb : rbufname\n");
    return 1;
  }
  SharedEventBuffer* buf = new SharedEventBuffer();
  buf->open(argv[1], 10000000);
  SharedEventBuffer::Header header;
  while (true) {
    buf->lock();
    memcpy(&header, buf->getHeader(), sizeof(SharedEventBuffer::Header));
    buf->unlock();
    B2INFO("write = " << header.nword_in * 4 * 1.e-6 << " MB, " <<
           "read = " << header.nword_out * 4 * 1.e-6 << " MB " <<
           header.count_in << " " << header.count_out);
    sleep(2);
  }
  return 0;
}

