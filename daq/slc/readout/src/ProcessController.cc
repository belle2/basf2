#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/readout/ProcessListener.h"
#include "daq/slc/readout/LogListener.h"

#include "daq/slc/system/Executor.h"
#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/StringUtil.h"
#include <daq/slc/base/Debugger.h>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace Belle2;

bool ProcessController::init(const std::string& name_in,
                             int nreserved)
{
  _name = (name_in.size() > 0) ? name_in : _callback->getNode()->getName();
  LogFile::open(_name);
  if (!_info.open(_name, nreserved, true)) {
    return false;
  }
  _info.init();
  return true;
}

void ProcessController::clear()
{
  _info.clear();
}

bool ProcessController::load(int timeout)
{
  _info.clear();
  _fork.cancel();
  int iopipe[2];
  if (pipe(iopipe) < 0) {
    perror("pipe");
  }
  _fork = Fork(new ProcessSubmitter(this, iopipe));
  PThread(new LogListener(this, iopipe));
  PThread(new ProcessListener(this));
  close(iopipe[1]);
  if (timeout > 0) {
    if (!_info.waitRunning(timeout)) {
      _callback->setReply("Failed to boot " + _name);
      return false;
    }
  }
  return true;
}

bool ProcessController::start()
{
  _info.lock();
  _info.setExpNumber(_callback->getExpNumber());
  _info.setColdNumber(_callback->getColdNumber());
  _info.setHotNumber(_callback->getHotNumber());
  _info.setNodeId(_callback->getNode()->getData()->getId());
  if (_info.getState() != RunInfoBuffer::RUNNING) {
    _callback->setReply(_name + " is not running");
    _info.unlock();
    return false;
  }
  _info.unlock();
  return true;
}

bool ProcessController::stop()
{
  return true;
}

bool ProcessController::abort()
{
  _info.clear();
  _fork.cancel();
  return true;
}

void ProcessSubmitter::run()
{
  close(1);
  dup2(_iopipe[1], 1);
  close(2);
  dup2(_iopipe[1], 2);
  close(_iopipe[0]);
  Executor executor;
  if (_con->getExecutable().size() == 0) {
    _con->setExecutable("basf2");
  }
  executor.setExecutable(_con->getExecutable());
  for (size_t i = 0; i < _con->_arg_v.size(); i++) {
    executor.addArg(_con->_arg_v[i]);
  }
  if (_con->getExecutable() == "basf2") {
    executor.addArg("--no-stats");
  }
  executor.execute();
}

