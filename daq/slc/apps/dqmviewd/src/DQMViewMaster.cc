#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"

#include <daq/slc/system/Fork.h>
#include <daq/slc/system/Executor.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

bool DQMViewMaster::boot()
{
  ConfigFile config("dqm");
  const std::string dumppath = config.get("DQM_DUMP_PATH");
  const std::string mappath = config.get("DQM_MAP_PATH");
  LogFile::debug("DQM_DUMP_PATH=%s", dumppath.c_str());
  LogFile::debug("DQM_MAP_PATH=%s", mappath.c_str());
  _fork_v = std::vector<Fork>();
  _fork_v.resize(_reader_v.size());
  for (size_t i = 0; i < _reader_v.size(); i++) {
    const std::string filename = _reader_v[i].getFileName();
    LogFile::debug("booting hserver %d %s", _port_v[i], filename.c_str());
    _fork_v[i] = Fork(new HSeverExecutor(_port_v[i], filename, mappath));
  }
  return true;
}

bool DQMViewMaster::abort()
{
  for (size_t i = 0; i < _reader_v.size(); i++) {
    _fork_v[i].cancel();
  }
  return true;
}

void HSeverExecutor::run()
{
  Executor executor;
  executor.setExecutable("hserver");
  executor.addArg(Belle2::form("%d", _port));
  executor.addArg(_dir + "/" + _mapfile);
  executor.execute();
}
