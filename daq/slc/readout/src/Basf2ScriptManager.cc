
#include "daq/slc/readout/Basf2ScriptManager.h"
#include "daq/slc/readout/ProcessListener.h"

#include "daq/slc/system/Executor.h"

#include "daq/slc/nsm/RCCallback.h"

#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/Debugger.h"

using namespace Belle2;

Basf2ScriptManager::~Basf2ScriptManager()
{

}

void Basf2ScriptManager::setCallback(RCCallback* callback)
{
  _callback = callback;
  setNode(callback->getNode());
}

void Basf2ScriptManager::setNode(NSMNode* node)
{
  _node = node;
  _buf_path = "/run_info_buf_" + _node->getName();
  _fifo_path = "/tmp/run_log_fifo_" + _node->getName();
}

bool Basf2ScriptManager::create()
{
  SharedMemory::unlink(_buf_path);
  _msg.unlink(_fifo_path);
  _buf.open(_buf_path);
  _msg.create(_fifo_path, "r");
  return true;
}

bool Basf2ScriptManager::open()
{

  _buf.open(_buf_path);
  _msg.open(_fifo_path, "w");
  return true;
}

bool Basf2ScriptManager::close()
{
  _buf.close();
  _msg.close();
  return true;
}

bool Basf2ScriptManager::unlink()
{
  SharedMemory::unlink(_buf_path);
  _msg.unlink(_fifo_path);
  return true;
}

bool Basf2ScriptManager::load()
{
  _buf.clear();
  _fork.cancel();
  _fork = Fork(new Basf2ScriptWorker(this));
  _thread = PThread(new ProcessListener(_callback, _fork, _script));
  return true;
}

bool Basf2ScriptManager::start()
{
  _buf.lock();
  _buf.setExpNumber(_callback->getExpNumber());
  _buf.setColdNumber(_callback->getColdNumber());
  _buf.setHotNumber(_callback->getHotNumber());
  _buf.setNodeId(_node->getData()->getId());
  _buf.setState(1);
  _buf.unlock();
  try {
    SystemLog log = _msg.recieveLog();
    if (log.getPriority() == SystemLog::INFO) {
      return true;
    } else {
      Belle2::debug("Error on readout worker : %s", log.getMessage().c_str());
      _callback->setReply(log.getMessage());
    }
  } catch (const IOException& e) {
    Belle2::debug("Fifo IO error");
    _callback->setReply("Fifo IO error");
  }
  return true;
}

bool Basf2ScriptManager::stop()
{
  _buf.clear();
  return true;
}

void Basf2ScriptWorker::run()
{
  const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
  const char* belle2_sub = getenv("BELLE2_SUBDIR");
  Executor executor;
  executor.setExecutable("%s/bin/%s/basf2", belle2_path, belle2_sub);
  executor.addArg("%s/%s/%s", belle2_path,
                  _manager->_scriptdir.c_str(), _manager->_script.c_str());
  for (size_t i = 0; i < _manager->_arg_v.size(); i++)
    executor.addArg(_manager->_arg_v[i]);
  executor.addArg("--no-stats");
  executor.execute();
}
