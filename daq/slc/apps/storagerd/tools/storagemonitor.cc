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
#include <daq/slc/base/ConfigFile.h>

#include <daq/slc/apps/storagerd/storage_status.h>

#include <fstream>
#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("%s storage monitor [config]", argv[0]);
    return 1;
  }
  const std::string stornode = argv[1];
  const std::string node = argv[2];
  ConfigFile config("slowcontrol");
  if (argc > 4) config.read(argv[3]);
  NSMCommunicator* comm = new NSMCommunicator();
  comm->init(NSMNode(node),
             config.get("nsm.global.host"),
             config.getInt("nsm.global.port"));
  NSMData data(stornode + "_STATUS", "storage_status", 1);
  storage_status* info = (storage_status*)data.open(comm);

  while (true) {
    sleep(2);
    fputs("\033[2J\033[0;0H", stdout);
    rewind(stdout);
    ftruncate(1, 0);
    printf(" exp = %04u run = %04u\n", info->expno, info->runno);
    printf(" # of files: %4u, # of bytes : %4.2f [GB]\n", info->nfiles, info->nbytes / 1024);
    printf(" rxqueue from eb2rx    : %4.1f [kB]\n", (float)(info->io[0].nqueue / 1024.));
    printf(" data in input  buffer : %4.1f [kB]\n", (float)(info->io[1].nqueue * 4 / 1024.));
    printf(" data in record buffer : %4.1f [kB]\n", (float)(info->io[2].nqueue * 4 / 1024.));
    printf("\n");
    printf(" %13s |      count | freq [kHz] | rate [MB/s] | evtsize [kB]\n", "node");
    for (int i = 0; i < 14; i++) {
      storage_status::io_status& nio(info->io[i]);
      if (i == 4 || i == 5) continue;
      std::string name = "basf2";
      if (i == 0 || i == 1) {
        name = "input";
      } else if (i == 2 || i == 3) {
        name = "record";
      } else {
        name = StringUtil::form("basf2_%d", i / 2);
      }
      if (i % 2 == 0) name += " (in)";
      else name += " (out)";
      if (nio.freq == 0) {
        printf("\x1b[49m\x1b[31m");
      } else {
        printf("\x1b[49m\x1b[32m");
      }
      printf(" %-13s | %s | %10s | %11s | %12s\x1b[49m\x1b[39m\n",
             name.c_str(),
             StringUtil::form("%10u", nio.count).c_str(),
             StringUtil::form("%02.2f", nio.freq).c_str(),
             StringUtil::form("%04.2f", nio.rate).c_str(),
             StringUtil::form("%03.2f", nio.evtsize).c_str());
    }
    printf("\n");
    for (int i = 0; i < 11; i++) {
      float available = 100. - info->diskusage[i];
      if (available <= 10) {
        printf("\x1b[49m\x1b[31m");
      } else {
        printf("\x1b[49m\x1b[32m");
      }
      printf(" disk%02d : available %5s [%%] (disk size = %4.2f [TB])\x1b[49m\x1b[39m\n",
             i + 1, StringUtil::form("%3.1f", available).c_str(),
             info->disksize[i] / 1024.);
    }
  }
  return 0;
}

