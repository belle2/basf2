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

#include <daq/slc/readout/ronode_status.h>

#include <fstream>
#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    LogFile::debug("%s monnode hostname portnode1 node2..", argv[0]);
    return 1;
  }
  const std::string monnode = argv[1];
  const std::string hostname = argv[2];
  const int port = atoi(argv[3]);
  ConfigFile config("slowcontrol");
  NSMCommunicator* comm = new NSMCommunicator();
  comm->init(NSMNode(monnode), hostname, port);
  std::vector<NSMData*> data_v;
  const std::string node = argv[4];
  NSMData data(node + "_STATUS", "ronode_status",
               ronode_status_revision);
  data.open(comm, true);

  while (true) {
    sleep(2);
    for (size_t i = 0; i < data_v.size(); i++) {
      data.update();
      fputs("\033[2J\033[0;0H", stdout);
      rewind(stdout);
      ftruncate(1, 0);
      ronode_status* info = (ronode_status*)data.get();
      printf(" exp = %04u run = %04u\n", info->expno, info->runno);
      printf(" rxqueue from eb0    : %4.1f [kB]\n", (float)(info->nqueue_in / 1024.));
      printf(" txqueue to eb1tx    : %4.1f [kB]\n", (float)(info->nqueue_out / 1024.));
      printf("\n");
      printf(" %15s |      count | freq [kHz] | rate [MB/s] | evtsize [kB]\n", "socket");
      std::string name = "";
      if (info->connection_in != 1) {
        name = "\x1b[49m\x1b[31m eb0  -x-> ropc";
      } else {
        name = "\x1b[49m\x1b[32m eb0  ---> ropc";
      }
      printf(" %-15s | %s | %10s | %11s | %12s\x1b[49m\x1b[39m\n",
             name.c_str(),
             StringUtil::form("%10u", info->nevent_in).c_str(),
             StringUtil::form("%02.2f", info->evtrate_in).c_str(),
             StringUtil::form("%04.2f", info->flowrate_in).c_str(),
             StringUtil::form("%03.2f", info->evtsize_in).c_str());
      if (info->connection_out != 1) {
        name = "\x1b[49m\x1b[31m ropc -x-> eb0";
      } else {
        name = "\x1b[49m\x1b[32m ropc ---> eb0";
      }
      printf(" %-15s | %s | %10s | %11s | %12s\x1b[49m\x1b[39m\n",
             name.c_str(),
             StringUtil::form("%10u", info->nevent_out).c_str(),
             StringUtil::form("%02.2f", info->evtrate_out).c_str(),
             StringUtil::form("%04.2f", info->flowrate_out).c_str(),
             StringUtil::form("%03.2f", info->evtsize_out).c_str());
    }
  }
  return 0;
}

