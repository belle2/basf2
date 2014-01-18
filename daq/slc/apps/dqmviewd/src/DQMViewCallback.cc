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
  PThread(new DQMPackageUpdater(this, _master));
}

bool DQMViewCallback::boot() throw()
{
  _master->boot();
  return true;
}

bool DQMViewCallback::start() throw()
{
  NSMMessage& nsm(getMessage());
  _expno = nsm.getParam(0);
  _runno = nsm.getParam(1);
  return true;
}

bool DQMViewCallback::stop() throw()
{
  _master->setRunNumbers(_expno, _runno);
  return true;
}

bool DQMViewCallback::abort() throw()
{
  return _master->abort();
}

