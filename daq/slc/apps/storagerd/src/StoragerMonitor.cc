#include "daq/slc/apps/storagerd/StoragerMonitor.h"

#include "daq/slc/apps/storagerd/storage_info_all.h"

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/base/ConfigFile.h>

#include "daq/storage/storage_info.h"

#include <unistd.h>
#include <cstring>

using namespace Belle2;

void StoragerMonitor::run()
{
  NSMData* nsm = new NSMData("STORAGE_INFO", "storage_info_all", 1);
  /*
  storage_info_all* data = (storage_info_all*)nsm->allocate(_callback->getCommunicator());
  memset(data, 0, sizeof(storage_info_all));
  while (true) {
    sleep(1);
    for (size_t i = 0; i < _callback->getNControllers(); i++) {
      ProcessController& con(_callback->getController(i));
      if (con.getInfo().isAvailable()) {
  storage_info* info = (storage_info*)con.getInfo().getReserved();
  data->nodeid[i] = info->nodeid;
  data->expno[i] = info->expno;
  data->runno[i] = info->runno;
  data->subno[i] = info->subno;
  data->stime[i] = info->stime;
  data->ctime[i] = info->ctime;
  data->count[i] = info->count;
  data->nbyte[i] = info->nbyte;
  data->freq[i] = info->freq;
  data->evtsize[i] = info->evtsize;
  data->rate[i] = info->rate;
      }
    }
  }
  */
}
