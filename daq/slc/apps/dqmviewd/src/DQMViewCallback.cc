#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"
#include "daq/slc/apps/dqmviewd/DQMPackageUpdater.h"

#include <daq/slc/system/Inotify.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/ConfigFile.h>

#include <sys/types.h>
#include <sys/stat.h>

using namespace Belle2;

void DQMViewCallback::init() throw()
{
  _master->init();
  _master->setState(State::INITIAL_S);
  PThread(new DQMPackageUpdater(this, _master));
}

bool DQMViewCallback::boot() throw()
{
  _master->boot();
  _master->setState(State::CONFIGURED_S);
  return true;
}

bool DQMViewCallback::load() throw()
{
  _master->setState(State::READY_S);
  return true;
}

bool DQMViewCallback::start() throw()
{
  NSMMessage& nsm(getMessage());
  _expno = nsm.getParam(0);
  _runno = nsm.getParam(1);
  _master->setRunNumbers(_expno, _runno);
  _master->setState(State::RUNNING_S);
  return true;
}

bool DQMViewCallback::stop() throw()
{
  Belle2::debug("creating new DQM records for run # %04d.%06d",
                (int)_expno, (int)_runno);
  ConfigFile config("dqm");
  const std::string dumppath = config.get("DQM_DUMP_PATH");
  std::vector<DQMFileReader>& reader_v(_master->getReaders());
  for (size_t i = 0; i < reader_v.size(); i++) {
    reader_v[i].dump(dumppath, _expno, _runno);
  }
  _master->setState(State::READY_S);
  return true;
}

bool DQMViewCallback::pause() throw()
{
  _master->setState(State::PAUSED_S);
}

bool DQMViewCallback::abort() throw()
{
  _master->setState(State::INITIAL_S);
  return _master->abort();
}

bool DQMViewCallback::recover() throw()
{
  return abort() && boot() && load();
}

