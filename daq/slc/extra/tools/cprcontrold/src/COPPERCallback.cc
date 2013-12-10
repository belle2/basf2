#include "COPPERCallback.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

#include <unistd.h>
#include <iostream>

using namespace Belle2;

COPPERCallback::COPPERCallback(NSMNode* node)
  : RCCallback(node)
{
  node->setData(new DataObject());
  _man.setCallback(this);
}

COPPERCallback::~COPPERCallback() throw()
{
}

void COPPERCallback::init() throw()
{
  _man.create();
}

bool COPPERCallback::boot() throw()
{
  download();
  DataObject* data = _node->getData();
  size_t length;
  bool* used_v = data->getBoolArray("hslb_used", length);
  for (size_t slot = 0; slot < length; slot++) {
    if (used_v[slot]) {
      DataObject* hslb = data->getObject(Belle2::form("hslb_%c", (char)slot));
      if (hslb != NULL) _hslbcon_v[slot].boot(slot, hslb);
    }
  }
  return true;
}

bool COPPERCallback::load() throw()
{
  bool boot_firm = (_confno == (int)getMessage().getParam(0));
  _confno = getMessage().getParam(0);
  download();
  int flag = 0;
  DataObject* data = _node->getData();
  size_t length;
  bool* used_v = data->getBoolArray("hslb_used", length);
  for (size_t slot = 0; slot < length; slot++) {
    if (used_v[slot]) {
      flag += 1 << slot;
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
  _man.clearArguments();
  const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
  _man.setScriptDir(Belle2::form("%s/daq/rawdata/examples/", belle2_path));
  _man.setScript(data->getText("script"));
  _man.addArgument(data->getText("host"));
  _man.addArgument(Belle2::form("%d", (int)_node->getID()));
  _man.addArgument(Belle2::form("%d", flag));
  _man.addArgument("1");
  _man.addArgument(_node->getName());
  return _man.load();
}

bool COPPERCallback::start() throw()
{
  return _man.start();
}

bool COPPERCallback::stop() throw()
{
  return _man.stop();
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
  return _man.stop();
}
