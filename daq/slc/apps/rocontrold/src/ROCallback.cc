#include "daq/slc/apps/rocontrold/ROCallback.h"

#include "daq/slc/apps/rocontrold/RecieverManager.h"
#include "daq/slc/apps/rocontrold/EventBuilderManager.h"

#include "daq/slc/apps/ProcessListener.h"

#include "daq/slc/system/Fork.h"

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

using namespace Belle2;

ROCallback::ROCallback(NSMNode* node)
  : RCCallback(node), _node(node)
{
  node->setData(new DataObject());
}

ROCallback::~ROCallback() throw()
{
}

void ROCallback::init() throw()
{
  _buf_path = "/run_info_buf_" + _node->getName();
  _fifo_path = "/run_log_fifo_" + _node->getName();
  SharedMemory::unlink(_buf_path);
  _msg.unlink(_fifo_path);
  _buf.open(_buf_path);
  _msg.create(_fifo_path);
}

bool ROCallback::boot() throw()
{
  return true;
}

bool ROCallback::load() throw()
{
  download();
  _buf.clear();
  system("killall basf2");
  _fork.cancel();
  _fork = Fork(new RecieverManager(_node, _buf_path, _fifo_path));
  _thread = PThread(new ProcessListener(this, _fork, "Serializer"));
  return true;
}

bool ROCallback::start() throw()
{
  _buf.lock();
  _buf.setExpNumber(getExpNumber());
  _buf.setColdNumber(getColdNumber());
  _buf.setHotNumber(getHotNumber());
  _buf.setNodeId(_node->getData()->getId());
  _buf.setState(1);
  _buf.unlock();
  return true;
}

bool ROCallback::stop() throw()
{
  _buf.clear();
  return true;
}

bool ROCallback::resume() throw()
{
  return true;
}

bool ROCallback::pause() throw()
{
  return true;
}

bool ROCallback::abort() throw()
{
  _fork.cancel();
  return true;
}

