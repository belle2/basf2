#include "daq/slc/apps/rocontrold/Stream0Controller.h"

#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void Stream0Controller::readDB(const DBObject& obj, int& port,
                               std::string& host, std::string& script)
{
  const DBObject& cobj(obj("stream0"));
  const DBObject& ccobj(cobj("sender", m_id - 2));
  port = ccobj.getInt("port");
  host = ccobj.getText("host");
  script = cobj.getText("script");
}

Stream0Controller::~Stream0Controller() throw()
{
}

void Stream0Controller::initArguments(const DBObject& obj)
{
  const int id = m_id - 2;
  int port = 0;
  std::string host, script;
  readDB(obj, port, host, script);
  std::string vname = StringUtil::form("stream0.sender[%d]", id);
  m_callback->add(new NSMVHandlerInt(vname + ".port", true, true, port));
  m_callback->add(new NSMVHandlerText(vname + ".host", true, true, host));
  m_callback->add(new NSMVHandlerText(vname + ".script", true, true, script));
}

bool Stream0Controller::loadArguments(const DBObject& obj)
{
  int port = 0;
  std::string host, script;
  readDB(obj, port, host, script);
  int used = 0;
  try {
    const std::string vname = StringUtil::form("%s", host.c_str());
    m_callback->get(m_callback->getRC().getName(), vname + ".used", used);
  } catch (const RCHandlerException& e) {
    LogFile::error(e.what());
  }
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
  return used > 0;
}

void Stream0Controller::check() throw()
{
  try {
    int used = 0;
    std::string host;
    std::string vname = StringUtil::form("stream0.sender[%d]", m_id - 2);
    m_callback->get(vname + ".host", host);
    vname = StringUtil::form("%s", host.c_str());
    m_callback->get(m_callback->getRC(), vname + ".used", used);
  } catch (const std::exception& e) {
    LogFile::warning("timeout");
  }
}

