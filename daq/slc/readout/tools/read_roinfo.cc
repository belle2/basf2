#include <daq/slc/readout/RunInfoBuffer.h>
//#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Date.h>

#include <unistd.h>

using namespace Belle2;

int main(int argc, const char** argv)
{
  if (argc < 2) {
    printf("Usage : %s <nodename>\n", argv[0]);
    return 1;
  }
  RunInfoBuffer status;
  status.open(argv[1]);
  event_header& header(status.getEventHeader());
  while (true) {
    printf("\n");
    printf("########################\n");
    printf("expno       : %u\n", (header.exp_run >> 22));
    printf("runno       : %u\n", (header.exp_run & 0x3FFF00) >> 8);
    printf("subno       : %u\n", (header.exp_run & 0xFF));
    printf("evtno       : %u\n", header.evtno);
    printf("ctime       : %u\n", (header.ctime_trgtype & 0x7FFFFFF0) >> 4);
    printf("trgtype     : %u\n", (header.ctime_trgtype & 0xF));
    printf("utime       : %u\n", header.utime);
    printf("date(utime) : %s\n", Date(header.utime).toString());
    printf("nodeid      : %u\n", header.nodeid);
    printf("crc_err     : %u\n", header.crc_err);
    printf("########################\n");
    sleep(1);
  }
  return 0;
}
