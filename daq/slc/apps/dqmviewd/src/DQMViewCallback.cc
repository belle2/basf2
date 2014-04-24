#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"
#include "daq/slc/apps/dqmviewd/DQMPackageUpdater.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

void DQMViewCallback::init() throw()
{
  _master.setCallback(this);
  _master.init();
  _master.boot();
  _master.setState(RCState::INITIAL_S);
  PThread(new DQMPackageUpdater(this, _master));
}

bool DQMViewCallback::boot() throw()
{
  _master.setState(RCState::CONFIGURED_S);
  return true;
}

bool DQMViewCallback::load() throw()
{
  _master.setState(RCState::READY_S);
  return true;
}

bool DQMViewCallback::start() throw()
{
  NSMMessage& nsm(getMessage());
  _expno = nsm.getParam(0);
  _runno = nsm.getParam(1);
  _master.setRunNumbers(_expno, _runno);
  _master.setState(RCState::RUNNING_S);
  return true;
}

bool DQMViewCallback::stop() throw()
{
  LogFile::debug("creating new DQM records for run # %04d.%06d",
                 (int)_expno, (int)_runno);
  ConfigFile config("dqm");
  const std::string dumppath = config.get("DQM_DUMP_PATH");
  std::vector<DQMFileReader>& reader_v(_master.getReaders());
  for (size_t i = 0; i < reader_v.size(); i++) {
    reader_v[i].dump(dumppath, _expno, _runno);
  }
  _master.setState(RCState::READY_S);
  return true;
}

bool DQMViewCallback::pause() throw()
{
  _master.setState(RCState::PAUSED_S);
  return true;
}

bool DQMViewCallback::abort() throw()
{
  _master.setState(RCState::INITIAL_S);
  return true;//_master.abort();
}

bool DQMViewCallback::recover() throw()
{
  return abort() && boot() && load();
}

