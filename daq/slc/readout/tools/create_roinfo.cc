#include <daq/slc/readout/RunInfoBuffer.h>
//#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Date.h>

#include <unistd.h>

using namespace Belle2;

int main(int argc, const char** argv)
{
  if (argc < 3) {
    printf("Usage : %s <nodename> <nodeid>\n", argv[0]);
    return 1;
  }
  RunInfoBuffer status;
  status.open(argv[1], atoi(argv[2]), true);
  status.getEventHeader();
  return 0;
}
