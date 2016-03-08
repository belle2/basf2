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
  const DBObjectList& objs(obj.getObjects("stream0"));
  used = obj("stream0").getBool("used");
  nsenders = (int)objs.size();
}

void EB0Controller::initArguments(const DBObject& obj)
{
  int port = 0, nsenders = 0;
  std::string executable;
  bool used = false;
  readDB(obj, port, executable, used, nsenders);
  m_callback->add(new NSMVHandlerInt("nstream0", true, false, nsenders));
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
    const DBObject& o_stream0(obj("stream0", i));
    const int port = o_stream0.getInt("port");
    const std::string nodename = StringUtil::tolower(o_stream0.hasText("name") ? o_stream0.getText("name") : o_stream0.getText("host"));
    int used = 1;
    try {
      m_callback->get(m_callback->getRC(), nodename + ".used", used, 5);
    } catch (const std::exception& e) {
      m_callback->log(LogFile::ERROR, "error on reading %s.used %s", nodename.c_str(), e.what());
    }
    if (used) {
      m_con.addArgument("%s:%d", (stream0_used ? "localhost" : nodename.c_str()), port);
    }
  }
  return true;
}
