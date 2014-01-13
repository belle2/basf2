#include "daq/slc/apps/storagerd/StoragerMonitor.h"

#include "daq/storage/storagein_data.h"
#include "daq/storage/storager_data.h"
#include "daq/storage/storageout_data.h"

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>
#include <cstring>

using namespace Belle2;

void StoragerMonitor::run()
{
  ConfigFile config("envmon");
  const std::string config_path = config.get("ENV_CONFIG_PATH");
  config.read(config_path + "/storage.conf");
  std::string nsmdata_name = config.get("ENV_NSMDATA_NAME");
  std::string nsmdata_format = config.get("ENV_NSMDATA_FORMAT");
  const int nsmdata_revision = config.getInt("ENV_NSMDATA_REVISION");
  NSMData* nsm = new NSMData(nsmdata_name, nsmdata_format, nsmdata_revision);
  storager_data* nsm_data = (storager_data*)nsm->allocate(_callback->getCommunicator());
  memset(nsm_data, 0, sizeof(storager_data));
  storager_data* data = NULL;
  ProcessController& con(_callback->getController(1));
  while (true) {
    sleep(1);
    if (data == NULL && con.getInfo().getParams() != NULL) {
      data = (storager_data*)con.getInfo().getReserved();
    }
    if (data != NULL) {
      con.getInfo().lock();
      if (data->nevts > nsm_data->nevts) {
        memcpy(nsm_data, data, sizeof(storager_data));
      }
      con.getInfo().unlock();
    }
  }
}
