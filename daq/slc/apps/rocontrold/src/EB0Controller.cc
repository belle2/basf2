#include "daq/slc/apps/rocontrold/EB0Controller.h"

#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

EB0Controller::~EB0Controller() throw()
{
}

bool EB0Controller::initArguments(const DBObject& obj) throw()
{
  try {
    const DBObject& cobj(obj("eb0"));
    int port = cobj.getInt("port");
    std::string executable = cobj.getText("executable");
    const DBObjectList& objs(obj("stream0").getObjects("sender"));
    bool used = obj("stream0").getBool("used");
    m_callback->add(new NSMVHandlerInt("stream0.used", true, true, (int)used));
    m_callback->add(new NSMVHandlerInt("stream0.nsenders", true, false, (int)objs.size()));
    m_callback->add(new NSMVHandlerInt("eb0.port", true, true, port));
    m_callback->add(new NSMVHandlerText("eb0.executable", true, true, executable));
    return true;
  } catch (const std::out_of_range& e) {
    LogFile::error("Bad DB input: %s", e.what());
  }
  return false;
}

void EB0Controller::loadArguments() throw()
{
  int stream0_used = 1, nsenders = 0, port = 0, used = 0, excluded = 0;
  std::string executable, host;
  m_callback->get("stream0.used", stream0_used);
  m_callback->get("stream0.nsenders", nsenders);
  m_callback->get("eb0.port", port);
  m_callback->get("eb0.executable", executable);
  m_con.setExecutable(executable);
  m_con.addArgument("-l");
  m_con.addArgument(port);
  for (int i = 0; i < nsenders; i++) {
    std::string vname = StringUtil::form("stream0.sender[%d]", i);
    m_callback->get(vname + ".port", port);
    m_callback->get(vname + ".host", host);
    vname = StringUtil::form("%s", host.c_str());
    m_callback->get(m_callback->getRC().getName(), vname + ".used", used);
    m_callback->get(m_callback->getRC().getName(), vname + ".excluded", excluded);
    if (used && !excluded) {
      m_con.addArgument("%s:%d", (stream0_used ? "localhost" : host.c_str()), port);
    }
  }
}
