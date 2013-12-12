#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/readout/ProcessLogListener.h"
#include "daq/slc/readout/ProcessListener.h"

#include "daq/slc/system/Executor.h"

#include "daq/slc/base/StringUtil.h"
#include <daq/slc/base/Debugger.h>

#include <cstdlib>

using namespace Belle2;

bool ProcessController::init()
{
  _msg.setNode(_callback->getNode()->getName(),
               _callback->getNode()->getData()->getId());
  _state = State::CONFIGURED_S;
  PThread(new ProcessLogListener(this));
  return _msg.create();
}

void ProcessController::clear()
{
  _msg.getInfo().clear();
  _msg.getLog().clear();
}

bool ProcessController::load(int timeout)
{
  _mutex.lock();
  _state = State::LOADING_TS;
  _msg.getInfo().clear();
  _fork.cancel();
  if (_name.size() == 0) {
    _name = _callback->getNode()->getName();
  }
  _fork = Fork(new ProcessSubmitter(this));
  _thread = PThread(new ProcessListener(this));
  _cond.wait(_mutex, timeout);
  if (_state != State::READY_S) {
    _callback->setReply(Belle2::form("Failed to be ready: %s",
                                     getMessage().c_str()));
    _mutex.unlock();
    return false;
  }
  _mutex.unlock();
  _state = State::READY_S;
  return true;
}

bool ProcessController::start(int timeout)
{
  _mutex.lock();
  _state = State::STARTING_TS;
  _msg.getInfo().lock();
  _msg.getInfo().setExpNumber(_callback->getExpNumber());
  _msg.getInfo().setColdNumber(_callback->getColdNumber());
  _msg.getInfo().setHotNumber(_callback->getHotNumber());
  _msg.getInfo().setNodeId(_callback->getNode()->getData()->getId());
  _msg.getInfo().setState(1);
  _msg.getInfo().notify();
  _msg.getInfo().unlock();
  _cond.wait(_mutex, timeout);
  if (_state != State::RUNNING_S) {
    _callback->setReply(Belle2::form("Failed to start run: %s",
                                     getMessage().c_str()));
    _mutex.unlock();
    return false;
  }
  _mutex.unlock();
  return true;
}

bool ProcessController::stop(int timeout)
{
  _mutex.lock();
  _state = State::STOPPING_TS;
  _msg.getInfo().clear();
  _cond.wait(_mutex, timeout);
  if (_state != State::READY_S) {
    _callback->setReply(Belle2::form("Failed to stop run: %s",
                                     getMessage().c_str()));
    _mutex.unlock();
    return false;
  }
  _mutex.unlock();
  return true;
}

bool ProcessController::abort()
{
  _msg.getInfo().clear();
  _msg.getLog().clear();
  _fork.cancel();
  return true;
}

void ProcessSubmitter::run()
{
  //close(1);
  //close(2);
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

