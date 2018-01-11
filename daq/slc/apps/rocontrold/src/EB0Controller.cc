#include "daq/slc/apps/rocontrold/EB0Controller.h"

#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

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
  m_eb_stat = NULL;
}

void EB0Controller::initArguments(const DBObject& obj)
{
  int port = 0;
  m_nsenders = 0;
  std::string executable;
  bool used = false;
  readDB(obj, port, executable, used, m_nsenders);
  m_callback->add(new NSMVHandlerInt("nstream0", true, false, m_nsenders));
  m_callback->add(new NSMVHandlerInt("eb0.port", true, true, port));
  m_callback->add(new NSMVHandlerText("eb0.executable", true, true, executable));
  std::string upname = std::string("/dev/shm/") + "eb0_up";
  std::string downname = std::string("/dev/shm/") + "eb0_down";
  if (m_eb_stat) delete m_eb_stat;
  m_eb_stat = new eb_statistics(upname.c_str(), m_nsenders, downname.c_str(), 1);
  for (int i = 0; i < m_nsenders; i++) {
    std::string vname = StringUtil::form("ebstat.in[%d].", i);
    m_callback->add(new NSMVHandlerInt(vname + "addr", true, false, 0));
    m_callback->add(new NSMVHandlerInt(vname + "port", true, false, 0));
    m_callback->add(new NSMVHandlerInt(vname + "connection", true, false, 0));
    m_callback->add(new NSMVHandlerInt(vname + "byte", true, false, 0));
    m_callback->add(new NSMVHandlerInt(vname + "event", true, false, 0));
    m_callback->add(new NSMVHandlerFloat(vname + "total_byte", true, false, 0));
    m_callback->add(new NSMVHandlerFloat(vname + "flowrate", true, false, 0));
    m_callback->add(new NSMVHandlerFloat(vname + "nqueue", true, false, 0));
    //m_callback->add(new NSMVHandlerFloat(vname + "nevent", true, false, 0));
    //m_callback->add(new NSMVHandlerFloat(vname + "evtrate", true, false, 0));
  }
  std::string vname = StringUtil::form("ebstat.out.");
  m_callback->add(new NSMVHandlerInt(vname + "addr", true, false, 0));
  m_callback->add(new NSMVHandlerInt(vname + "port", true, false, 0));
  m_callback->add(new NSMVHandlerInt(vname + "connection", true, false, 0));
  m_callback->add(new NSMVHandlerInt(vname + "byte", true, false, 0));
  m_callback->add(new NSMVHandlerInt(vname + "event", true, false, 0));
  m_callback->add(new NSMVHandlerFloat(vname + "total_byte", true, false, 0));
  m_callback->add(new NSMVHandlerFloat(vname + "flowrate", true, false, 0));
  m_callback->add(new NSMVHandlerFloat(vname + "nqueue", true, false, 0));
  // m_callback->add(new NSMVHandlerFloat(vname + "nevent", true, false, 0));
  //m_callback->add(new NSMVHandlerFloat(vname + "evtrate", true, false, 0));
}

bool EB0Controller::loadArguments(const DBObject& obj)
{
  if (obj("eb0").getBool("xinetd")) {
    setUsed(false);
    return false;
  }
  setUsed(true);
  int port = 0;
  m_nsenders = 0;
  std::string executable;
  bool stream0_used = false;
  readDB(obj, port, executable, stream0_used, m_nsenders);
  m_con.setExecutable(executable);
  std::string upname = std::string("/dev/shm/") + "eb0_up";
  std::string downname = std::string("/dev/shm/") + "eb0_down";
  if (m_eb_stat) delete m_eb_stat;
  m_eb_stat = new eb_statistics(upname.c_str(), m_nsenders, downname.c_str(), 1);
  m_con.addArgument("-u");
  m_con.addArgument(upname);
  m_con.addArgument("-d");
  m_con.addArgument(downname);
  m_con.addArgument("-l");
  m_con.addArgument(port);
  for (int i = 0; i < m_nsenders; i++) {
    const DBObject& o_stream0(obj("stream0", i));
    const int port = o_stream0.getInt("port");
    const std::string nodename = StringUtil::tolower(o_stream0.hasText("name") ?
                                                     o_stream0.getText("name") : o_stream0.getText("host"));
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
  memset(m_nevent_in, 0, sizeof(m_nevent_in));
  memset(m_nevent_in, 0, sizeof(m_nevent_out));
  memset(m_total_byte_in, 0, sizeof(m_total_byte_in));
  memset(m_total_byte_in, 0, sizeof(m_total_byte_out));
  m_t0 = Time().get();
  return true;
}

void EB0Controller::check()
{
  double t1 = Time().get();
  double dt = t1 - m_t0;
  const RCState state(m_callback->getNode().getState());
  if (state == RCState::RUNNING_S || state == RCState::READY_S) {
    std::vector<IOInfo> ioinfo;
    IOInfo info;
    info.setLocalAddress((int)m_eb_stat->down(0).addr);
    info.setLocalPort((int)m_eb_stat->down(0).port);
    ioinfo.push_back(info);
    for (int i = 0; i < m_nsenders; i++) {
      IOInfo info;
      info.setLocalAddress((int)m_eb_stat->up(i).addr);
      info.setLocalPort((int)m_eb_stat->up(i).port);
      ioinfo.push_back(info);
    }
    IOInfo::checkTCP(ioinfo);
    std::string vname = StringUtil::form("ebstat.out.");
    m_callback->set(vname + "event", (int)m_eb_stat->down(0).event);
    m_callback->set(vname + "byte", (float)(m_eb_stat->down(0).byte / 1024.));
    m_callback->set(vname + "addr", (int)m_eb_stat->down(0).addr);
    m_callback->set(vname + "port", (int)m_eb_stat->down(0).port);
    m_callback->set(vname + "connection", (int)m_eb_stat->down(0).port > 0);
    m_callback->set(vname + "nqueue", ioinfo[0].getTXQueue());
    unsigned long long total_byte = m_eb_stat->down(0).total_byte;
    double dbyte = total_byte - m_total_byte_out[0];
    double flowrate = dbyte / dt / 1024 / 1024; //MB
    m_callback->set(vname + "total_byte", (float)total_byte);
    m_callback->set(vname + "flowrate", (float)flowrate);
    m_total_byte_out[0] = total_byte;
    for (int i = 0; i < m_nsenders; i++) {
      std::string vname = StringUtil::form("ebstat.in[%d].", i);
      m_callback->set(vname + "event", (int)m_eb_stat->up(i).event);
      m_callback->set(vname + "byte", (float)(m_eb_stat->up(i).byte / 1024.));
      m_callback->set(vname + "addr", (int)m_eb_stat->up(i).event);
      m_callback->set(vname + "port", (int)m_eb_stat->up(i).port);
      m_callback->set(vname + "connection", (int)m_eb_stat->up(i).port > 0);
      m_callback->set(vname + "nqueue", ioinfo[i + 1].getRXQueue());
      total_byte = m_eb_stat->up(i).total_byte;
      dbyte = total_byte - m_total_byte_in[i];
      flowrate = dbyte / dt / 1024 / 1024; //MB
      m_callback->set(vname + "total_byte", (float)total_byte);
      m_callback->set(vname + "flowrate", (float)flowrate);
      m_total_byte_in[i] = total_byte;
      //m_nevent_in[i] = nevent;
    }
  } else {
    std::string vname = StringUtil::form("ebstat.out.");
    m_callback->set(vname + "event", 0);
    m_callback->set(vname + "byte", 0);
    m_callback->set(vname + "addr", 0);
    m_callback->set(vname + "port", 0);
    m_callback->set(vname + "connection", 0);
    m_callback->set(vname + "nqueue", 0);
    //m_callback->set(vname + "nevent", 0);
    //m_callback->set(vname + "evtrate", 0);
    m_callback->set(vname + "total_byte", 0);
    m_callback->set(vname + "flowrate", 0);
    for (int i = 0; i < m_nsenders; i++) {
      std::string vname = StringUtil::form("ebstat.in[%d].", i);
      m_callback->set(vname + "event", 0);
      m_callback->set(vname + "byte", 0);
      m_callback->set(vname + "addr", 0);
      m_callback->set(vname + "port", 0);
      m_callback->set(vname + "connection", 0);
      m_callback->set(vname + "nqueue", 0);
      //m_callback->set(vname + "nevent", 0);
      //m_callback->set(vname + "evtrate", 0);
      m_callback->set(vname + "total_byte", 0);
      m_callback->set(vname + "flowrate", 0);
    }
  }
  m_t0 = t1;
}
