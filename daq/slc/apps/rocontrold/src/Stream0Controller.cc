#include "daq/slc/apps/rocontrold/Stream0Controller.h"

#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

Stream0Controller::~Stream0Controller() throw()
{
}

void Stream0Controller::initArguments(const DBObject& obj)
{
  const DBObject& cobj(obj("stream0", m_id - 2));
  m_host = cobj.getText("host");
}

bool Stream0Controller::loadArguments(const DBObject& obj)
{
  const DBObject& cobj(obj("stream0", m_id - 2));
  int used = cobj.getBool("used");
  if (!used) {
    setUsed(false);
    return false;
  }
  int port = cobj.getInt("port");
  m_host = cobj.getText("host");
  std::string script = cobj.getText("script");
  try {
    m_callback->get(m_callback->getRC().getName(), m_host + ".used", used);
  } catch (const RCHandlerException& e) {
    LogFile::error(e.what());
  }
  setUsed(used);
  if (used) {
    m_con.setExecutable("basf2");
    m_con.addArgument(StringUtil::form("%s/%s", getenv("BELLE2_LOCAL_DIR"),
                                       script.c_str()));
    m_con.addArgument(m_host);
    m_con.addArgument("1");
    m_con.addArgument(port);
    const std::string nodename = StringUtil::tolower(m_callback->getNode().getName());
    m_con.addArgument(nodename + "_" + m_name);
  }
  return used > 0;
}

void Stream0Controller::check() throw()
{
  try {
    int used = 0;
    m_callback->get(m_callback->getRC(), m_host + ".used", used);
  } catch (const std::exception& e) {
    LogFile::warning("timeout");
  }
}

