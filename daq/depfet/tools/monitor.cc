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
  if (argc > 3) config.read(argv[3]);
  NSMCommunicator com;
  com.init(NSMNode(node), config.get("nsm.host"), config.getInt("nsm.port"));
  //NSMData data(stornode, "storage_status", storage_status_revision);
  NSMData data(stornode, "storage", storage_status_revision);
  storage_status* info = (storage_status*)data.open(com);
  while (true) {
    sleep(2);
    fputs("\033[2J\033[0;0H", stdout);
    rewind(stdout);
    ftruncate(1, 0);
    printf(" exp = %04u run = %04u\n", info->expno, info->runno);
    printf(" # of files: %4u, # of bytes : %4.2f [GB]\n", info->nfiles, info->nbytes / 1024);
    printf(" connection to eb2rx   : %s \x1b[49m\x1b[39m\n",
           ((info->node[0].connection_in == 1) ? "\x1b[49m\x1b[32mEstablished" : "\x1b[49m\x1b[31mNot ready"));
    //printf(" rxqueue from eb2rx    : %4.1f [kB]\n", (float)(info->node[0].nqueue_in / 1024.));
    //printf(" data in input  buffer : %4.1f [kB]\n", (float)(info->node[0].nqueue_out * 4 / 1024.));
    //printf(" data in record buffer : %4.1f [kB]\n", (float)(info->node[1].nqueue_out * 4 / 1024.));
    printf(" rxqueue from eb2rx    : %4.1f [kB]\n", (float)(info->node[0].nqueue_in));
    printf(" data in input  buffer : %4.1f [kB]\n", (float)(info->node[0].nqueue_out));
    printf(" data in record buffer : %4.1f [kB]\n", (float)(info->node[1].nqueue_out));
    printf("\n");
    printf(" %13s | in |      count | freq [kHz] | rate [MB/s] | evtsize [kB]", "node");
    printf(" | out |      count | freq [kHz] | rate [MB/s] | evtsize [kB]\n");
    for (int i = 0; i < 5; i++) {
      storage_status::node_status& node(info->node[i]);
      if (i == 2) continue;
      std::string name = "basf2";
      if (i == 0) {
        name = "input";
      } else if (i == 1) {
        name = "record";
      } else {
        name = StringUtil::form("basf2_%d", i / 2);
      }
      printf(" %-13s ", name.c_str());
      if (node.evtrate_in == 0) {
        printf("\x1b[49m\x1b[31m");
      } else {
        printf("\x1b[49m\x1b[32m");
      }
      printf("|    | %s | %10s | %11s | %12s \x1b[49m\x1b[39m",
             StringUtil::form("%10u", node.nevent_in).c_str(),
             StringUtil::form("%02.2f", node.evtrate_in).c_str(),
             StringUtil::form("%04.2f", node.flowrate_in).c_str(),
             StringUtil::form("%03.2f", node.evtsize_in).c_str());
      if (node.evtrate_out == 0) {
        printf("\x1b[49m\x1b[31m");
      } else {
        printf("\x1b[49m\x1b[32m");
      }
      printf("|     | %s | %10s | %11s | %12s\x1b[49m\x1b[39m\n",
             StringUtil::form("%10u", node.nevent_out).c_str(),
             StringUtil::form("%02.2f", node.evtrate_out).c_str(),
             StringUtil::form("%04.2f", node.flowrate_out).c_str(),
             StringUtil::form("%03.2f", node.evtsize_out).c_str());
    }
    printf("\n");
    for (int i = 0; i < 3; i++) {
      float available = 100. - info->disk[i].available;
      if (available <= 10) {
        printf("\x1b[49m\x1b[31m");
      } else {
        printf("\x1b[49m\x1b[32m");
      }
      printf(" disk%02d : available %5s [%%] (disk size = %4.2f [TB])\x1b[49m\x1b[39m\n",
             i + 1, StringUtil::form("%3.1f", available).c_str(),
             info->disk[i].size / 1024.);
    }
  }
  return 0;
}

