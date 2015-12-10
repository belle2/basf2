#include "daq/slc/apps/rocontrold/Stream1Controller.h"

#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

void Stream1Controller::readDB(const DBObject& obj, int& port,
                               std::string& script)
{
  const DBObject& cobj(obj("stream1"));
  port = cobj.getInt("port");
  script = cobj.getText("script");
}

void Stream1Controller::initArguments(const DBObject& obj)
{
  int port = 0;
  std::string script;
  readDB(obj, port, script);
  m_callback->add(new NSMVHandlerInt("stream1.port", true, true, port));
  m_callback->add(new NSMVHandlerText("stream1.script", true, true, script));
}

bool Stream1Controller::loadArguments(const DBObject& obj)
{
  int port = 0;
  std::string script;
  readDB(obj, port, script);
  m_con.setExecutable("basf2");
  m_con.addArgument(StringUtil::form("%s/%s", getenv("BELLE2_LOCAL_DIR"),
                                     script.c_str()));
  m_con.addArgument("1");
  m_con.addArgument(port);
  const std::string nodename = StringUtil::tolower(m_callback->getNode().getName());
  m_con.addArgument(nodename + "_" + m_name);
  return true;
}

