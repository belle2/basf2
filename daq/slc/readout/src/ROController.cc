#include "daq/slc/readout/ROController.h"
#include "daq/slc/readout/ROMessageMonitor.h"
#include "daq/slc/readout/ProcessListener.h"

#include "daq/slc/system/Executor.h"

#include "daq/slc/base/StringUtil.h"
#include <daq/slc/base/Debugger.h>

#include <cstdlib>

using namespace Belle2;

bool ROController::init()
{
  _msg.setNode(_callback->getNode()->getName(),
               _callback->getNode()->getData()->getId());
  _state = State::CONFIGURED_S;
  _thread_msg = PThread(new ROMessageMonitor(this));
  return _msg.create();
}

void ROController::clear()
{
  _msg.getInfo().clear();
  _msg.getLog().clear();
}

bool ROController::load(int timeout)
{
  _mutex.lock();
  _state = State::LOADING_TS;
  _msg.getInfo().clear();
  _fork.cancel();
  _fork = Fork(new ROSubmitter(this));
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

bool ROController::start(int timeout)
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

bool ROController::stop(int timeout)
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

bool ROController::abort()
{
  _msg.getInfo().clear();
  _msg.getLog().clear();
  _fork.cancel();
  return true;
}

void ROSubmitter::run()
{
  //close(1);
  //close(2);
  const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
  const char* belle2_sub = getenv("BELLE2_SUBDIR");
  Executor executor;
  executor.setExecutable("%s/bin/%s/basf2", belle2_path, belle2_sub);
  executor.addArg("%s/%s/%s", belle2_path,
                  _con->_scriptdir.c_str(), _con->_script.c_str());
  for (size_t i = 0; i < _con->_arg_v.size(); i++)
    executor.addArg(_con->_arg_v[i]);
  executor.addArg("--no-stats");
  executor.execute();
}

