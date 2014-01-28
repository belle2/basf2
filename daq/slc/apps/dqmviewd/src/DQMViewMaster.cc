#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"

#include <daq/slc/system/Fork.h>
#include <daq/slc/system/Executor.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void DQMViewMaster::init()
{
  for (size_t i = 0; i < _reader_v.size(); i++) {
    _con_v.push_back(ProcessController());
  }
  for (size_t i = 0; i < _reader_v.size(); i++) {
    const std::string name = _reader_v[i].getName();
    _con_v[i].setCallback(_callback);
    _con_v[i].init("hserver_" + name);
  }
}

bool DQMViewMaster::boot()
{
  ConfigFile config("dqm");
  const std::string dumppath = config.get("DQM_DUMP_PATH");
  const std::string mappath = config.get("DQM_MAP_PATH");
  LogFile::debug("DQM_DUMP_PATH=%s", dumppath.c_str());
  LogFile::debug("DQM_MAP_PATH=%s", mappath.c_str());
  for (size_t i = 0; i < _reader_v.size(); i++) {
    const std::string filename = _reader_v[i].getFileName();
    LogFile::debug("booting hserver %d %s", _port_v[i], filename.c_str());
    _con_v[i].clearArguments();
    _con_v[i].setExecutable("hserver");
    _con_v[i].addArgument(Belle2::form("%d", _port_v[i]));
    _con_v[i].addArgument(filename);
    _con_v[i].load(-1);
  }
  return true;
}

bool DQMViewMaster::abort()
{
  for (size_t i = 0; i < _reader_v.size(); i++) {
    _con_v[i].abort();
  }
  return true;
}

