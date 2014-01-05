#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/readout/ProcessLogListener.h"
#include "daq/slc/readout/ProcessListener.h"
#include "daq/slc/readout/StdOutListener.h"

#include "daq/slc/system/Executor.h"

#include "daq/slc/base/StringUtil.h"
#include <daq/slc/base/Debugger.h>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace Belle2;

bool ProcessController::init(const std::string& name_in)
{
  _name = (name_in.size() > 0) ? name_in : _callback->getNode()->getName();
  _msg.setNode(_name, _callback->getNode()->getData()->getId());
  PThread(new ProcessLogListener(this));
  return _msg.create();
}

void ProcessController::clear()
{
  _msg.clear();
}

bool ProcessController::load(int timeout)
{
  _msg.getInfo().clear();
  _fork.cancel();
  if (_name.size() == 0) {
    _name = _callback->getNode()->getName();
  }
  _fork = Fork(new ProcessSubmitter(this));
  _thread = PThread(new ProcessListener(this));
  if (timeout > 0) {
    if (_msg.waitRunning(timeout)) {
      _callback->setReply(std::string("Failed to start ") + _name);
      return false;
    }
  }
  return true;
}

bool ProcessController::start()
{
  _msg.getInfo().lock();
  _msg.getInfo().setExpNumber(_callback->getExpNumber());
  _msg.getInfo().setColdNumber(_callback->getColdNumber());
  _msg.getInfo().setHotNumber(_callback->getHotNumber());
  _msg.getInfo().setNodeId(_callback->getNode()->getData()->getId());
  if (_msg.getState() != RunInfoBuffer::RUNNING) {
    _callback->setReply(_name + " is not running");
    _msg.getInfo().unlock();
    return false;
  }
  _msg.getInfo().unlock();
  return true;
}

bool ProcessController::stop()
{
  _msg.getInfo().clear();
  return true;
}

bool ProcessController::abort()
{
  _msg.clear();
  _fork.cancel();
  return true;
}

void ProcessSubmitter::run()
{
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

