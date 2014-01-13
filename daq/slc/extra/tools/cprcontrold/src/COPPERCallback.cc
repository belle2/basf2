#include "COPPERCallback.h"

#include "daq/slc/xml/XMLParser.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/ConfigFile.h"

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

COPPERCallback::COPPERCallback(NSMNode* node, const std::string& configname)
  : RCCallback(node)
{
  node->setData(new DataObject());
  node->setState(State::INITIAL_S);
  _con.setCallback(this);
  ConfigFile config(configname);
  _path = config.get("COPPER_BASF2_SCRIPT");
  _hostname = config.get("COPPER_HOST_" + node->getName());
}

COPPERCallback::~COPPERCallback() throw()
{
}

void COPPERCallback::init() throw()
{
  _con.init("basf2");
}

void COPPERCallback::term() throw()
{
  _con.abort();
  _con.getInfo().unlink();
}

bool COPPERCallback::boot() throw()
{
  download();
  ConfigFile config;
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
  bool boot_firm = (_confno == (int)getMessage().getParam(0));
  _confno = getMessage().getParam(0);
  ConfigFile config;
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
        LogFile::error("Failed to boot HSLB:%c", (char)(slot + 'a'));
        setReply(Belle2::form("Failed to boot HSLB:%c", (char)(slot + 'a')));
        return false;
      }
      if (!_hslbcon_v[slot].load()) {
        LogFile::error("Failed to load HSLB:%c", (char)(slot + 'a'));
        return false;
      }
    }
  }
  _con.clearArguments();
  _con.addArgument(_path);
  _con.addArgument(_hostname);
  _con.addArgument(Belle2::form("%d", (int)_node->getData()->getId()));
  _con.addArgument(Belle2::form("%d", flag));
  _con.addArgument("1");
  _con.addArgument("basf2");
  if (_con.load(30)) {
    LogFile::debug("load succeded");
  } else {
    LogFile::debug("load timeout");
  }
  return true;
}

bool COPPERCallback::start() throw()
{
  return _con.start();
}

bool COPPERCallback::stop() throw()
{
  return _con.stop();
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
  return (abort() && boot() && load());
}

bool COPPERCallback::abort() throw()
{
  return _con.abort();
}
