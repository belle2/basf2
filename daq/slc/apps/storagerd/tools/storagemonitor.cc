#include "daq/slc/readout/ProcessController.h"

#include "daq/storage/storage_info.h"

#include "daq/slc/nsm/NSMData.h"
#include "daq/slc/apps/storagerd/storage_info_all.h"

#include <daq/slc/system/Time.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <sstream>

using namespace Belle2;

int main(int argc, char** argv)
{
  /*  const int interval = (argc > 1) ? atoi(argv[1]) : 2;
  ConfigFile config("storage");
  config.cd("default");
  std::vector<RunInfoBuffer> info_v;
  info_v.resize(3 + config.getInt("record_nproc"));
  info_v[0].open("storagein", sizeof(storage_info) / sizeof(int));
  info_v[1].open("storagerecord", sizeof(storage_info) / sizeof(int));
  info_v[2].open("storageout", sizeof(storage_info) / sizeof(int));
  for (size_t i = 3; i < info_v.size(); i++) {
    info_v[i].open(StringUtil::form("basf2_%d", i - 3), sizeof(storage_info) / sizeof(int));
  }

  storage_info_all* sinfo_out = new storage_info_all;
  storage_info* sinfo_v = new storage_info[info_v.size()];
  sinfo_out->nnodes = info_v.size();
  for (size_t i = 0; i < info_v.size(); i++) {
    storage_info* sinfo = (storage_info*)info_v[i].getReserved();
    storage_info* sinfo1 = &sinfo_v[i];
    memcpy(sinfo1, sinfo, sizeof(storage_info));
  }

  while (true) {
    sleep(interval);
    for (size_t i = 0; i < info_v.size(); i++) {
      storage_info* sinfo = (storage_info*)info_v[i].getReserved();
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
    }

    fputs("\033[2J\033[0;0H", stdout);
    rewind(stdout);
    ftruncate(1, 0);
    std::stringstream ss;
    ss << " Updated at " << Date().toString() << std::endl
       << "---------------------------------------------------------------------------------" << std::endl
       << StringUtil::form("%15s | %10s | %10s | %10s | %10s ",
                           "Node", "Count", "Freq [kHz]", "Rate [MB/s]", "Event size [kB/event]") << std::endl
       << "---------------------------------------------------------------------------------" << std::endl;
    for (size_t i = 0; i < info_v.size(); i++) {
      ss << StringUtil::form("%15s | %10d | %10f | %10f | %10f ",
                             info_v[i].getName().c_str(), sinfo_out->count[i],
                             sinfo_out->freq[i], sinfo_out->rate[i], sinfo_out->evtsize[i]) << std::endl;
      if (i == 2) {
        ss << "---------------------------------------------------------------------------------" << std::endl;
      }
    }
    ss << "---------------------------------------------------------------------------------" << std::endl
       << std::endl;
    fputs(ss.str().c_str(), stdout);
  }
  */
  return 0;
}
