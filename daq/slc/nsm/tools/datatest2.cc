#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMDataStore.h>

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
  NSMCommunicator com(argv[1]);
  NSMData data("RODATA1", "mybuf", mybuf_revision);
  mybuf* status = NULL;
  while (status == NULL) {
    try {
      status = (mybuf*)data.open(&com);
    } catch (const NSMHandlerException& e) {
      LogFile::error(e.what());
      status = NULL;
    }
  }
  while (true) {
    data.update();
    LogFile::debug("mybuf.io[0].count = %u", status->io[0].count);
    sleep(1);
  }

  return 0;
}
