#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/readout/mybuf.h>

#include <daq/slc/system/LogFile.h>

#include <unistd.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 1) {
    LogFile::debug("Usage : %s ", argv[0]);
    return 1;
  }
  NSMData data("RODATA1", "mybuf", mybuf_revision);
  mybuf* status = (mybuf*)data.allocate(NULL);
  unsigned int count = 0;
  while (true) {
    status->io[0].count = ++count;
    LogFile::debug("mybuf.io[0].count = %u", status->io[0].count);
    sleep(1);
  }

  return 0;
}
