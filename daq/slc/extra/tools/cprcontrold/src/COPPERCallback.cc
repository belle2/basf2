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
  m_con.setCallback(this);
  ConfigFile config(configname);
  m_path = config.get("copper.basf2.dir") + "/";
}

COPPERCallback::~COPPERCallback() throw()
{
}

void COPPERCallback::init() throw()
{
  m_con.init("basf2");
}

void COPPERCallback::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
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
      m_hslbcon_v[i].reset();
      if (!m_hslbcon_v[i].load(hslb_v[i])) {
        return false;
      }
    }
  }
  m_con.clearArguments();
  m_con.addArgument(m_path + copper.getText("basf2script"));
  m_con.addArgument(copper.getText("hostname"));
  m_con.addArgument(StringUtil::form("%d", copper.getInt("copper_id")));
  m_con.addArgument(StringUtil::form("%d", flag));
  m_con.addArgument("1");
  m_con.addArgument("basf2");
  if (m_con.load(30)) {
    LogFile::debug("load succeded");
    return true;
  }
  LogFile::debug("load timeout");
  return false;
}

bool COPPERCallback::start() throw()
{
  return m_con.start();
}

bool COPPERCallback::stop() throw()
{
  return m_con.stop();
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
  m_con.abort();
  getNode().setState(RCState::INITIAL_S);
  return true;
}
