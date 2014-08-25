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
#include <daq/slc/base/Date.h>

#include <daq/slc/apps/rfarm/rfunitinfo.h>

#include <fstream>
#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("%s rfarm monitor [config]", argv[0]);
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
  NSMData data(stornode + "_STATUS", "rfunitinfo", rfunitinfo_revision);
  rfunitinfo* info = (rfunitinfo*)data.open(comm);

  while (true) {
    sleep(2);
    fputs("\033[2J\033[0;0H", stdout);
    rewind(stdout);
    ftruncate(1, 0);
    printf(" nnodes = %u update time = %s\n", info->nnodes, Date(info->updatetime).toString());
    printf("\n");
    printf(" node #  | sysstate |      nevent | nqueue | error |  flow rate | event size | event rate |   load ave\n");
    for (int i = 0; i < MAX_NODES; i++) {
      rfunitinfo::rfnodeinfo& node(info->nodeinfo[i]);
      printf(" %d (in)  | %8d | %12d | %6d | %5d | %10s | %10s | %10s | %10s \n",
             i, node.sysstate, node.nevent_in, node.nqueue_in, node.error,
             StringUtil::form("%02.2f", node.flowrate_in).c_str(),
             StringUtil::form("%02.2f", node.avesize_in).c_str(),
             StringUtil::form("%02.2f", node.evtrate_in).c_str(),
             StringUtil::form("%02.2f", node.loadave).c_str());
      printf(" %d (out) | -------- | %12d | %6d | ----- | %10s | %10s | %10s | ----------- \n",
             i, node.nevent_out, node.nqueue_out,
             StringUtil::form("%02.2f", node.flowrate_out).c_str(),
             StringUtil::form("%02.2f", node.avesize_out).c_str(),
             StringUtil::form("%02.2f", node.evtrate_out).c_str());
    }
    printf("\n");
  }
  return 0;
}

