#include "COPPERCallback.h"

#include "daq/slc/xml/XMLParser.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/ConfigFile.h"

#include <unistd.h>
#include <cstdlib>
#include <iostream>

using namespace Belle2;

COPPERCallback::COPPERCallback(NSMNode* node, const std::string& rc_config)
  : RCCallback(node), _rc_config(rc_config)
{
  node->setData(new DataObject());
  node->setState(State::INITIAL_S);
  _con.setCallback(this);
}

COPPERCallback::~COPPERCallback() throw()
{
}

void COPPERCallback::init() throw()
{
  _con.init(_node->getName());
  ConfigFile(_rc_config);
}

bool COPPERCallback::boot() throw()
{
  Belle2::debug("BOOT");
  download();
  ConfigFile config("cdc");
  XMLParser parser;
  DataObject* data = _node->getData();
  size_t length;
  bool* used_v = data->getBoolArray("hslb_used", length);
  for (size_t slot = 0; slot < length; slot++) {
    if (used_v[slot]) {
      std::string hslb_name = Belle2::form("hslb_%c", (char)(slot + 'a'));
      DataObject* hslb = data->getObject(hslb_name);
      if (hslb != NULL) {
        if (_el_v[slot] == NULL) {
          _el_v[slot] = parser.parse(config.get("RC_XML_PATH") + "/" +
                                     hslb->getClassName() + ".xml");
        }
        if (_el_v[slot] != NULL)
          _hslbcon_v[slot].boot(slot, hslb, _el_v[slot]);
      }
    }
  }
  return true;
}

bool COPPERCallback::load() throw()
{
  Belle2::debug("LOAD");
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
  _con.clearArguments();
  _con.addArgument(Belle2::form("%s/daq/rawdata/examples/",
                                getenv("BELLE2_LOCAL_DIR"))
                   + data->getText("script"));
  _con.addArgument(data->getText("host"));
  _con.addArgument(Belle2::form("%d", (int)_node->getData()->getId()));
  _con.addArgument(Belle2::form("%d", flag));
  _con.addArgument("1");
  _con.addArgument(_node->getName());
  if (_con.load(5)) {
    Belle2::debug("(DEBUG) load succeded");
  } else {
    Belle2::debug("(DEBUG) load timeout");
  }
  return true;
}

bool COPPERCallback::start() throw()
{
  Belle2::debug("START");
  if (_con.start(5)) {
    Belle2::debug("(DEBUG) start succeded");
  } else {
    Belle2::debug("(DEBUG) start timeout");
  }
  return true;
}

bool COPPERCallback::stop() throw()
{
  Belle2::debug("STOP");
  return _con.stop(-1);
}

bool COPPERCallback::resume() throw()
{
  return true;
}

bool COPPERCallback::pause() throw()
{
  return true;
}

bool COPPERCallback::recover() throw()
{
  Belle2::debug("RECOVER");
  return (_con.abort() && load());
}

bool COPPERCallback::abort() throw()
{
  Belle2::debug("ABORT");
  return _con.abort();
}
