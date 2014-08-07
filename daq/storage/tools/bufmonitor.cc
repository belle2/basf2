//+
// File : flow_monitor.cc
// Description : Readout flow rate monitor
//
// Author : Tomoyuki Konno, Tokyo Metroplitan University
// Date : 25 - Sep - 2013
//-

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <daq/slc/apps/storagerd/storage_info_all.h>

#include <fstream>
#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("%s storage monitor", argv[0]);
    return 1;
  }
  const std::string stornode = argv[1];
  const std::string node = argv[2];
  NSMCommunicator* comm = new NSMCommunicator();
  comm->init(NSMNode(node));
  NSMData data(stornode + "_STATUS", "storage_info_all", 1);
  storage_info_all* info = (storage_info_all*)data.open(comm);

  while (true) {
    sleep(2);
    fputs("\033[2J\033[0;0H", stdout);
    rewind(stdout);
    ftruncate(1, 0);
    printf("exp = %04u run = %04u\n", info->expno, info->runno);
    printf("nfile = %4u nbytes = %4.2f [MB]\n", info->nfiles, info->nbytes);
    printf("disk usage = %3.1f %% available disk size = %4.2f [TB]\n",
           (100. - info->diskusage), info->disksize / 1024.);
    printf("nqueue | count | freq [kHz] | evtsize [kB] | rate [MB/s]\n");
    for (int i = 0; i < 14; i++) {
      storage_info_all::io_status& nio(info->io[i]);
      printf("%010u | %08u | %02.2f | %03.2f | %04.2f\n",
             nio.nqueue, nio.count, nio.freq, nio.evtsize, nio.rate);
    }
  }
  return 0;
}

