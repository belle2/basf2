#include "daq/slc/apps/rocontrold/Stream1Controller.h"

#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

bool Stream1Controller::initArguments(const DBObject& obj) throw()
{
  const DBObject& cobj(obj.getObject("stream1"));
  int port = cobj.getInt("port");
  std::string script = cobj.getText("script");
  m_callback->add(new NSMVHandlerInt("stream1.port", true, true, port));
  m_callback->add(new NSMVHandlerText("stream1.script", true, true, script));
  return true;
}

void Stream1Controller::loadArguments() throw()
{
  int port = 0;
  std::string script;
  m_callback->get("stream1.port", port);
  m_callback->get("stream1.script", script);
  m_con.setExecutable("basf2");
  m_con.addArgument(StringUtil::form("%s/%s", getenv("BELLE2_LOCAL_DIR"),
                                     script.c_str()));
  m_con.addArgument("1");
  m_con.addArgument(port);
  m_con.addArgument(m_name);
}

