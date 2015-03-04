#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/apps/storagerd/storage_status.h>

#include <daq/slc/base/ConfigFile.h>

#include <daq/slc/system/LogFile.h>

#include <unistd.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 1) {
    LogFile::debug("Usage : %s <dataname> <format> <revision> [incpath]", argv[0]);
    return 1;
  }
  NSMData data(argv[1], argv[2], atoi(argv[3]));
  data.parse(argc > 4 ? argv[4] : NULL);
  data.printPV();
  return 0;
}
