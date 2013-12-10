#include "COPPERCallback.h"

#include "SenderManager.h"

#include "daq/slc/apps/ProcessListener.h"

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/PThread.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <cstring>
#include <cstdio>
#include <iostream>

using namespace Belle2;

COPPERCallback::COPPERCallback(NSMNode* node)
  : RCCallback(node)
{
  node->setData(new DataObject());
  _listener = NULL;
}

COPPERCallback::~COPPERCallback() throw()
{
}

void COPPERCallback::init() throw()
{
  _buf_path = "/run_info_buf_" + _node->getName();
  _fifo_path = "/tmp/run_log_fifo_" + _node->getName();
  SharedMemory::unlink(_buf_path);
  _msg.unlink(_fifo_path);
  _buf.open(_buf_path);
  _msg.create(_fifo_path, "r");
}

bool COPPERCallback::boot() throw()
{
  download();
  for (int slot = 0; slot < 4; slot++) {
    DataObject* hslb = _node->getData()->getObject(Belle2::form("hslb_%c", (char)slot));
    if (hslb != NULL)
      _hslbcon_v[slot].boot(slot, hslb);
  }
  return true;
}

bool COPPERCallback::load() throw()
{
  bool boot_firm = (_confno == (int)getMessage().getParam(0));
  _confno = getMessage().getParam(0);
  download();
  _buf.clear();
  for (size_t slot = 0; slot < 4; slot++) {
    if (_node->getData()->getBool(Belle2::form("hslb_%c", (char)slot))) {
      _hslbcon_v[slot].setBootFirmware(boot_firm);
      if (!(_hslbcon_v[slot].reset() && _hslbcon_v[slot].boot())) {
        Belle2::debug("[ERROR] Failed to boot HSLB:%c", (char)(slot + 'a'));
        setReply(Belle2::form("Failed to boot HSLB:%c", (char)(slot + 'a')));
        return false;
      }
      if (!_hslbcon_v[slot].load()) {
        Belle2::debug("[ERROR] Failed to load HSLB:%c", (char)(slot + 'a'));
        return false;
      }
    }
  }
  if (_listener != NULL) {
    _listener->setRunning(false);
    _listener = NULL;
    _thread.cancel();
  }
  system("killall basf2");
  _fork.cancel();
  _fork = Fork(new SenderManager(_node));
  _listener = new ProcessListener(this, _fork, "basf2_readout");
  _thread = PThread(_listener);
  return true;
}

bool COPPERCallback::start() throw()
{
  _buf.lock();
  _buf.setExpNumber(getExpNumber());
  _buf.setColdNumber(getColdNumber());
  _buf.setHotNumber(getHotNumber());
  _buf.setNodeId(_node->getData()->getId());
  _buf.setState(1);
  _buf.unlock();
  try {
    SystemLog log = _msg.recieveLog();
    if (log.getPriority() == SystemLog::INFO) {
      return true;
    } else {
      Belle2::debug("Error on readout worker : %s", log.getMessage().c_str());
      setReply(log.getMessage());
    }
  } catch (const IOException& e) {
    Belle2::debug("Fifo IO error");
    setReply("Fifo IO error");
  }
  return false;
}

bool COPPERCallback::stop() throw()
{
  _buf.clear();
  return true;
}

bool COPPERCallback::resume() throw()
{
  return true;
}

bool COPPERCallback::pause() throw()
{
  return true;
}

bool COPPERCallback::abort() throw()
{
  _buf.clear();
  _fork.cancel();
  return true;
}
