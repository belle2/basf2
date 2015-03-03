#include "daq/slc/apps/rocontrold/Stream0Controller.h"

#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

Stream0Controller::~Stream0Controller() throw()
{
}

bool Stream0Controller::initArguments(const DBObject& obj) throw()
{
  const DBObject& cobj(obj.getObject("stream0"));
  const DBObject& ccobj(cobj.getObject("sender", m_id - 2));
  int port = ccobj.getInt("port");
  std::string host = ccobj.getText("host");
  std::string script = cobj.getText("script");
  int id = m_id - 2;
  std::string vname = StringUtil::form("stream0.sender[%d]", id);
  m_callback->add(new NSMVHandlerInt(vname + ".port", true, true, port));
  m_callback->add(new NSMVHandlerText(vname + ".host", true, true, host));
  m_callback->add(new NSMVHandlerText(vname + ".script", true, true, script));
  return true;
}

void Stream0Controller::loadArguments() throw()
{
  int used, excluded, port;
  std::string host, script;
  std::string vname = StringUtil::form("stream0.sender[%d]", m_id - 2);
  m_callback->get(vname + ".port", port);
  m_callback->get(vname + ".host", host);
  m_callback->get(vname + ".script", script);
  vname = StringUtil::form("%s", host.c_str());
  m_callback->get(m_callback->getRC().getName(), vname + ".used", used);
  m_callback->get(m_callback->getRC().getName(), vname + ".excluded", excluded);
  used &= !excluded;
  setUsed(used);
  if (used) {
    m_con.setExecutable("basf2");
    m_con.addArgument(StringUtil::form("%s/%s", getenv("BELLE2_LOCAL_DIR"),
                                       script.c_str()));
    m_con.addArgument(host);
    m_con.addArgument("1");
    m_con.addArgument(port);
    m_con.addArgument(m_name);
  }
}

void Stream0Controller::check() throw()
{
  try {
    int used = 0, excluded = 0;
    std::string host;
    std::string vname = StringUtil::form("stream0.sender[%d]", m_id - 2);
    m_callback->get(vname + ".host", host);
    vname = StringUtil::form("%s", host.c_str());
    if (m_callback->get(m_callback->getRC(), vname + ".used", used))
      LogFile::debug("%s is used? %s", host.c_str(), (used ? "true" : "false"));
    if (m_callback->get(m_callback->getRC(), vname + ".excluded", excluded))
      LogFile::debug("%s is excluded? %s", host.c_str(), (excluded ? "true" : "false"));
  } catch (const std::exception& e) {
    LogFile::warning("timeout");
  }
}

