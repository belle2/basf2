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
  int id = m_id - 2;
  const DBObject& cobj(obj("stream0", id));
  m_host = cobj.getText("host");
  std::string vname = StringUtil::form("rostat.out[%d].", id);
  m_callback->add(new NSMVHandlerInt(vname + "nevent", true, false, 0));;
  m_callback->add(new NSMVHandlerFloat(vname + "evtrate", true, false, 0));;
  m_callback->add(new NSMVHandlerFloat(vname + "evtsize", true, false, 0));;
  m_callback->add(new NSMVHandlerFloat(vname + "flowrate", true, false, 0));;
  m_callback->add(new NSMVHandlerInt(vname + "connection", true, false, 0));;
  m_callback->add(new NSMVHandlerFloat(vname + "nqueue", true, false, 0));;
  vname = StringUtil::form("rostat.in[%d].", id);
  m_callback->add(new NSMVHandlerInt(vname + "nevent", true, false, 0));;
  m_callback->add(new NSMVHandlerFloat(vname + "evtrate", true, false, 0));;
  m_callback->add(new NSMVHandlerFloat(vname + "evtsize", true, false, 0));;
  m_callback->add(new NSMVHandlerFloat(vname + "flowrate", true, false, 0));;
  m_callback->add(new NSMVHandlerInt(vname + "connection", true, false, 0));;
  m_callback->add(new NSMVHandlerFloat(vname + "nqueue", true, false, 0));;
}

bool Stream0Controller::loadArguments(const DBObject& obj)
{
  int id = m_id - 2;
  const DBObject& cobj(obj("stream0", id));
  int used = cobj.getBool("used");
  if (!used) {
    setUsed(false);
    return false;
  }
  int port = cobj.getInt("port");
  std::string nodename = StringUtil::tolower(cobj.hasText("name") ? cobj.getText("name") : cobj.getText("host"));
  m_host = StringUtil::tolower(cobj.getText("host"));
  std::string script = cobj.getText("script");
  try {
    LogFile::info(nodename + ".used");
    m_callback->get(m_callback->getRC(), nodename + ".used", used);
  } catch (const TimeoutException& e) {
    LogFile::error(e.what());
  } catch (const RCHandlerException& e) {
    LogFile::error(e.what());
  }
  setUsed(used);
  if (used) {
    m_con.setExecutable(StringUtil::form("%s/%s", getenv("BELLE2_LOCAL_DIR"), script.c_str()));
    m_con.addArgument(m_host);
    m_con.addArgument("1");
    m_con.addArgument(port);
    nodename = StringUtil::tolower(m_callback->getNode().getName());
    m_con.addArgument(nodename + "_" + m_name);
  }
  return used > 0;
}

void Stream0Controller::check()
{
  try {
    ronode_status& status(getFlow().monitor());
    int id = m_id - 2;
    const RCState state(m_callback->getNode().getState());
    if (state == RCState::RUNNING_S || state == RCState::READY_S) {
      std::string vname = StringUtil::form("rostat.out[%d].", id);
      m_callback->set(vname + "nevent", (int)status.nevent_out);
      m_callback->set(vname + "evtrate", status.evtrate_out);
      m_callback->set(vname + "evtsize", status.evtsize_out);
      m_callback->set(vname + "flowrate", status.flowrate_out);
      m_callback->set(vname + "connection", (int)status.connection_out);
      m_callback->set(vname + "nqueue", (float)status.nqueue_out);
      vname = StringUtil::form("rostat.in[%d].", id);
      m_callback->set(vname + "nevent", (int)status.nevent_in);
      m_callback->set(vname + "evtrate", status.evtrate_in);
      m_callback->set(vname + "evtsize", status.evtsize_in);
      m_callback->set(vname + "flowrate", status.flowrate_in);
      m_callback->set(vname + "connection", (int)status.connection_in);
      m_callback->set(vname + "nqueue", (float)status.nqueue_in);
    } else {
      std::string vname = StringUtil::form("rostat.out[%d].", id);
      m_callback->set(vname + "nevent", 0);
      m_callback->set(vname + "evtrate", 0);
      m_callback->set(vname + "evtsize", 0);
      m_callback->set(vname + "flowrate", 0);
      m_callback->set(vname + "connection", 0);
      m_callback->set(vname + "nqueue", 0);
      vname = StringUtil::form("rostat.in[%d].", id);
      m_callback->set(vname + "nevent", 0);
      m_callback->set(vname + "evtrate", 0);
      m_callback->set(vname + "evtsize", 0);
      m_callback->set(vname + "flowrate", 0);
      m_callback->set(vname + "connection", 0);
      m_callback->set(vname + "nqueue", 0);
    }
  } catch (const std::exception& e) {
    LogFile::warning("timeout");
  }
}

