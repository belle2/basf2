#include "COPPERCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

COPPERCallback::COPPERCallback(const NSMNode& node,
                               const std::string& configname)
  : RCCallback(node)
{
  _con.setCallback(this);
  ConfigFile config(configname);
  _path = config.get("copper.basf2.dir") + "/";
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
  return true;
}

bool COPPERCallback::load() throw()
{
  int flag = 0;
  const ConfigObject& copper(getConfig().getObject());
  const ConfigObjectList& hslb_v(copper.getObjects("hslb"));
  for (size_t i = 0; i < hslb_v.size(); i++) {
    if (hslb_v[i].getBool("used")) {
      flag += 1 << i;
      _hslbcon_v[i].reset();
      if (!_hslbcon_v[i].load(hslb_v[i])) {
        return false;
      }
    }
  }
  _con.clearArguments();
  _con.addArgument(_path + copper.getText("basf2script"));
  _con.addArgument(copper.getText("hostname"));
  _con.addArgument(StringUtil::form("%d", copper.getInt("copper_id")));
  _con.addArgument(StringUtil::form("%d", flag));
  _con.addArgument("1");
  _con.addArgument("basf2");
  if (_con.load(30)) {
    LogFile::debug("load succeded");
    return true;
  }
  LogFile::debug("load timeout");
  return false;
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
  if (abort() && boot() && load()) {
    getNode().setState(RCState::READY_S);
    return true;
  }
  return false;
}

bool COPPERCallback::abort() throw()
{
  _con.abort();
  getNode().setState(RCState::INITIAL_S);
  return true;
}
