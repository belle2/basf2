#include "daq/slc/apps/storagerd/StoragerMonitor.h"

#include "daq/slc/apps/storagerd/storage_info_all.h"

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/Time.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include "daq/storage/storage_info.h"

#include <unistd.h>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace Belle2;

int StoragerMonitor::checkConnection(const std::string& proc_name, int port)
{
  std::string cmd =
    StringUtil::form("/usr/sbin/lsof -a -c '%s' -i TCP:%d | "
                     "grep ESTABLISHED | wc -l",
                     proc_name.c_str(), port);
  FILE* file = popen(cmd.c_str(), "r");
  char str[1024];
  memset(str, '\0', 1024);
  fread(str, 1, 1024 - 1, file);
  pclose(file);
  std::string s = str;
  return atoi(s.c_str());
}


void StoragerMonitor::run()
{
  NSMNode& node(_callback->getNode());
  ConfigFile config("storage");
  int port_in = config.getInt("DATA_STORAGE_FROM_PORT");
  int port_out = config.getInt("DATA_STORAGE_TO_PORT");
  const int interval = 2;
  std::vector<RunInfoBuffer*> info_v;
  for (size_t i = 0; i < _callback->getControllers().size(); i++) {
    info_v.push_back(&(_callback->getController(i).getInfo()));
  }
  storage_info_all* sinfo_out = (storage_info_all*)(_callback->getData().get());
  storage_info* sinfo_v = new storage_info[info_v.size()];
  sinfo_out->nnodes = info_v.size();
  for (size_t i = 0; i < info_v.size(); i++) {
    storage_info* sinfo = (storage_info*)info_v[i]->getReserved();
    storage_info* sinfo1 = &sinfo_v[i];
    memcpy(sinfo1, sinfo, sizeof(storage_info));
  }
  while (true) {
    sleep(interval);
    for (size_t i = 0; i < info_v.size(); i++) {
      storage_info* sinfo = (storage_info*)info_v[i]->getReserved();
      storage_info* sinfo1 = &sinfo_v[i];
      memcpy(sinfo1, sinfo, sizeof(storage_info));
      unsigned long long ctime = Time().get();
      double length = ctime - sinfo_out->ctime[i];
      double dcount = sinfo1->count - sinfo_out->count[i];
      double dnbyte = sinfo1->nbyte - sinfo_out->nbyte[i];
      sinfo_out->freq[i] = dcount / length / 1000.;
      sinfo_out->evtsize[i] = dnbyte / dcount / 1000.;
      sinfo_out->rate[i] = dnbyte / length / 1000000.;
      sinfo_out->ctime[i] = ctime;
      sinfo_out->nodeid[i] = sinfo1->nodeid;
      sinfo_out->expno[i] = sinfo1->expno;
      sinfo_out->runno[i] = sinfo1->runno;
      sinfo_out->subno[i] = sinfo1->subno;
      sinfo_out->stime[i] = sinfo1->stime;
      sinfo_out->count[i] = sinfo1->count;
      sinfo_out->nbyte[i] = sinfo1->nbyte;
      if (dcount > 0) {
        sinfo_out->state[i] = 3;//running
      } else if (sinfo1->nodeid > 0) {
        sinfo_out->state[i] = 2;//ready
      } else {
        sinfo_out->state[i] = 0;//unknown
      }
    }
    sinfo_out->connection[0] = checkConnection("storagein", port_in);
    sinfo_out->connection[1] = checkConnection("storageout", port_out);
    if (sinfo_out->connection[0] == 0 && sinfo_v[0].nodeid > 0) {
      sinfo_out->state[0] = 1;//not ready
    }
    if (sinfo_out->connection[1] == 0 && sinfo_v[2].nodeid > 0) {
      sinfo_out->state[2] = 1;//not ready
    }
    sinfo_out->rcstate = node.getState().getId();
  }
}
