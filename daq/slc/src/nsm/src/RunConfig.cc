#include "RunConfig.h"

extern "C" {
#include <nsm2.h>
#include "run_config.h"
}

#include <cstring>

using namespace Belle2;

RunConfig::RunConfig(const std::string& data_name) throw()
  : NSMData(data_name, "run_config", run_config_revision)
{
  memset(_used_v, 0, sizeof(_used_v));
  _nused = 0;
  _run_type = "";
  _operators = "";
  _version = -1;
}

void RunConfig::read() throw(NSMHandlerException)
{
  const run_config* config = (const run_config*)get();
  _nused = config->nused;
  memcpy(_used_v, config->used, sizeof(config->used));
  _version = config->version;
  _run_type = config->run_type;
  _trigger_mode = config->trigger_mode;
  _dummy_rate = config->dummy_rate;
  _trigger_limit = config->trigger_limit;
}

void RunConfig::write() throw(NSMHandlerException)
{
  run_config* config = (run_config*)get();
  config->nused = _nused;
  memcpy(config->used, _used_v, sizeof(config->used));
  config->version = _version;
  strcpy(config->run_type, _run_type.c_str());
  config->trigger_mode = _trigger_mode;
  config->dummy_rate = _dummy_rate;
  config->trigger_limit = _trigger_limit;
}

