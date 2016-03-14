#include "daq/slc/apps/rocontrold/EB0Controller.h"

#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

EB0Controller::~EB0Controller() throw()
{
}

void EB0Controller::readDB(const DBObject& obj, int& port,
                           std::string& executable, bool& used, int& nsenders)
{
  const DBObject& o_eb0(obj("eb0"));
  port = o_eb0.getInt("port");
  executable = o_eb0.getText("executable");
  const DBObjectList& objs(obj("stream0").getObjects("sender"));
  used = obj("stream0").getBool("used");
  nsenders = (int)objs.size();
}

void EB0Controller::initArguments(const DBObject& obj)
{
  int port = 0, nsenders = 0;
  std::string executable;
  bool used = false;
  readDB(obj, port, executable, used, nsenders);
  m_callback->add(new NSMVHandlerInt("stream0.used", true, true, (int)used));
  m_callback->add(new NSMVHandlerInt("stream0.nsenders", true, false, nsenders));
  m_callback->add(new NSMVHandlerInt("eb0.port", true, true, port));
  m_callback->add(new NSMVHandlerText("eb0.executable", true, true, executable));
}

bool EB0Controller::loadArguments(const DBObject& obj)
{
  if (obj("eb0").getBool("xinetd")) {
    setUsed(false);
    return false;
  }
  setUsed(true);
  int port = 0, nsenders = 0;
  std::string executable;
  bool stream0_used = false;
  readDB(obj, port, executable, stream0_used, nsenders);
  m_con.setExecutable(executable);
  m_con.addArgument("-l");
  m_con.addArgument(port);
  for (int i = 0; i < nsenders; i++) {
    const DBObject& o_sender(obj("stream0").getObject("sender", i));
    const int port = o_sender.getInt("port");
    const std::string host = o_sender.getText("host");
    int used = 0;
    m_callback->get(m_callback->getRC().getName(), host + ".used", used);
    if (used) {
      m_con.addArgument("%s:%d", (stream0_used ? "localhost" : host.c_str()), port);
    }
  }
  return true;
}
