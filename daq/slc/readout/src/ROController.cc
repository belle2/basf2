#include "daq/slc/readout/ROController.h"
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
  return _msg.create();
}

bool ROController::load(int timeout)
{
  _msg.getInfo().clear();
  _fork.cancel();
  _fork = Fork(new ROSubmitter(this));
  _thread = PThread(new ProcessListener(_callback, _fork, _script));
  State state = wait(timeout);
  if (state != State::READY_S) {
    _callback->setReply(Belle2::form("Failed to be ready: state = %s",
                                     state.getLabel()));
    return false;
  }
  return true;
}

bool ROController::start(int timeout)
{
  _msg.getInfo().lock();
  _msg.getInfo().setExpNumber(_callback->getExpNumber());
  _msg.getInfo().setColdNumber(_callback->getColdNumber());
  _msg.getInfo().setHotNumber(_callback->getHotNumber());
  _msg.getInfo().setNodeId(_callback->getNode()->getData()->getId());
  _msg.getInfo().setState(1);
  _msg.getInfo().notify();
  _msg.getInfo().unlock();
  State state = wait(timeout);
  if (state != State::RUNNING_S) {
    _callback->setReply(Belle2::form("Failed to start run: state = %s",
                                     state.getLabel()));
    return false;
  }
  return true;
}

bool ROController::stop(int timeout)
{
  _msg.getInfo().clear();
  State state = wait(timeout);
  if (state != State::READY_S) {
    _callback->setReply(Belle2::form("Failed to start run: state = %s",
                                     state.getLabel()));
    return false;
  }
  return true;
}

bool ROController::abort()
{
  _msg.getInfo().clear();
  _fork.cancel();
  return true;
}

State ROController::wait(int timeout)
{
  int priority = 0;
  std::string message = _msg.getLog().recieve(priority, timeout);
  if (priority == RunLogMessanger::NOTICE) {
    return State(message);
  } else if (priority == RunLogMessanger::ERROR) {
    return State::ERROR_ES;
  } else if (priority == RunLogMessanger::FATAL) {
    return State::FATAL_ES;
  }
  return State::UNKNOWN;
}

void ROSubmitter::run()
{
  const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
  const char* belle2_sub = getenv("BELLE2_SUBDIR");
  Executor executor;
  //executor.setExecutable("%s/bin/%s/basf2", belle2_path, belle2_sub);
  executor.setExecutable("dummy_basf2");
  //executor.addArg("%s/%s/%s", belle2_path,
  //                _con->_scriptdir.c_str(), _con->_script.c_str());
  for (size_t i = 0; i < _con->_arg_v.size(); i++)
    executor.addArg(_con->_arg_v[i]);
  //executor.addArg("--no-stats");
  executor.execute();
}
