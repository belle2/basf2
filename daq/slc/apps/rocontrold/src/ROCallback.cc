#include "daq/slc/apps/rocontrold/ROCallback.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/readout/ronode_info.h"
#include "daq/slc/readout/ronode_status.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstring>
#include <sstream>

#include <unistd.h>

using namespace Belle2;

ROCallback::ROCallback(const NSMNode& node, const std::string& conf)
  : RCCallback(node), m_file(conf)
{
  system("killall basf2");
}

ROCallback::~ROCallback() throw()
{
}

void ROCallback::init() throw()
{
  m_data = NSMData(getNode().getName() + "_STATUS", "ronode_status",
                   ronode_status_revision);
  m_data.allocate(getCommunicator());
  const size_t nproc = m_file.getInt("ropc.nrecv0");
  m_con = std::vector<ProcessController>();
  for (size_t i = 0; i < 1 + nproc; i++) {
    m_con.push_back(ProcessController(this));
    m_flow.push_back(FlowMonitor());
  }
  m_con[0].init("recvStream1", 1);
  m_flow[0].open(&m_con[0].getInfo());
  for (size_t i = 1; i < m_con.size(); i++) {
    m_con[i].init(StringUtil::form("recvStream0_%d", i - 1), i);
    m_flow[i].open(&m_con[i].getInfo());
  }
}

void ROCallback::term() throw()
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
    m_con[i].getInfo().unlink();
  }
}

bool ROCallback::load() throw()
{
  if (m_con[0].isAlive()) return true;
  const char* path_b2lib = getenv("BELLE2_LOCAL_DIR");
  const DBObject& obj(getConfig().getObject());
  LogFile::debug("ROCallback::load %s %d", obj.getName().c_str(), obj.getId());
  std::stringstream ss;
  ss << "#!/bin/sh" << std::endl
     << "#Thu Oct 17 14:17:13 CEST 2013" << std::endl
     << "cd /home/usr/b2daq/eb" << std::endl
     << "killall eb0 > /dev/null 2>&1" << std::endl
     << "sleep 1" << std::endl
     << "./eb0";
  const bool use_recv0 = m_file.getInt("ropc.nrecv0") > 0;
  m_eflag = m_reserved_i[0] = m_reserved_i[1] = 0;
  m_reserved_i[0] |= (use_recv0) & 0x01 << 0;
  m_reserved_i[0] |= 0x01 << 1;
  m_reserved_i[0] |= 0x01 << 2;
  m_reserved_i[0] |= 0x01 << 3;
  for (size_t i = 1; i < m_con.size(); i++) {
    const DBObject& cobj(obj.getObject("copper_from", i - 1));
    LogFile::debug("ROCallback::load %d %s", i, (cobj.getBool("used") ? "true" : "false"));
    if (cobj.getBool("used")) {
      m_reserved_i[0] |= 0x01 << (3 + i);
      if (use_recv0) {
        ss << " " << "127.0.0.1" << ":" << cobj.getInt("port");
      } else {
        ss << " " << cobj.getText("hostname") << ":" << m_file.getInt("ropc.copper.port");
      }
    }
  }
  ss << std::endl << "EOT" << std::endl;
  std::ofstream fout(m_file.get("ropc.eb0.script").c_str());
  fout << ss.str();
  fout.close();
  if (use_recv0) {
    const std::string script0 = obj.getText("ropc_script0");
    for (size_t i = 1; i < m_con.size(); i++) {
      const DBObject& cobj(obj.getObject("copper_from", i - 1));
      if (cobj.getBool("used")) {
        m_con[i].clearArguments();
        m_con[i].setExecutable("basf2");
        m_con[i].addArgument(StringUtil::form("%s/%s", path_b2lib, script0.c_str()));
        m_con[i].addArgument(cobj.getText("hostname"));
        m_con[i].addArgument("1");
        m_con[i].addArgument(StringUtil::form("%d", cobj.getInt("port")));
        m_con[i].addArgument(StringUtil::form("recvStream0_%d", i - 1));
        if (!m_con[i].load(10)) {
          LogFile::error("Failed to boot %d-th recvStream0", i - 3);
          return false;
        }
        LogFile::debug("Booted %d-th recvStream0", i - 3);
        usleep(10000);
      }
    }
  }
  const std::string script1 = obj.getText("ropc_script1");
  m_con[0].setExecutable("basf2");
  m_con[0].clearArguments();
  m_con[0].addArgument(StringUtil::form("%s/%s", path_b2lib, script1.c_str()));
  m_con[0].addArgument("1");
  m_con[0].addArgument(StringUtil::form("%d", obj.getInt("port_from")));
  m_con[0].addArgument("recvStream1");
  m_con[0].load(0);
  /*
  if (!m_con[0].load(10)) {
    LogFile::error("Failed to boot recvStream1");
    return false;
  }
  */
  LogFile::debug("Booted recvStream1");
  return true;
}

bool ROCallback::start() throw()
{
  ronode_status* status = (ronode_status*)m_data.get();
  status->stime = Time().getSecond();
  const bool use_recv0 = m_file.getBool("ropc.nrecv0") > 0;
  if (use_recv0) {
    for (size_t i = 0; i < m_con.size(); i++) {
      m_con[i].start();
    }
  } else {
    m_con[0].start();
  }
  return true;
}

bool ROCallback::stop() throw()
{
  ronode_status* status = (ronode_status*)m_data.get();
  status->stime = 0;
  return true;
}

bool ROCallback::resume() throw()
{
  return true;
}

bool ROCallback::pause() throw()
{
  return true;
}

bool ROCallback::recover() throw()
{
  if (abort() && load()) {
    getNode().setState(RCState::READY_S);
    return true;
  }
  return false;
}

bool ROCallback::abort() throw()
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
  }
  getNode().setState(RCState::NOTREADY_S);
  return true;
}

void ROCallback::timeout() throw()
{
  if (m_data.isAvailable()) {
    ronode_status* nsm = (ronode_status*)m_data.get();
    if (m_flow[0].isAvailable()) {
      ronode_status& status(m_flow[0].monitor());
      uint32 stime = nsm->stime;
      memcpy(nsm, &status, sizeof(ronode_status));
      nsm->stime = stime;
    }
    double loads[3];
    if (getloadavg(loads, 3) > 0) {
      nsm->loadavg = (float)loads[0];
    } else {
      nsm->loadavg = -1;
    }
    nsm->reserved_i[0] = m_reserved_i[0];
    nsm->reserved_i[1] = m_reserved_i[1];
    m_eflag = 0;
    for (size_t i = 1; i < m_con.size(); i++) {
      if (((m_reserved_i[0] >> (3 + i)) & 0x01) == 1) {
        ronode_status& status(m_flow[i].monitor());
        if (status.connection_in == 0) {
          m_eflag |= 0x01 << (3 + i);
        }
      }
    }
    nsm->eflag = m_eflag;
  }
  int eflag = m_con[0].getInfo().getErrorFlag();
  if (eflag > 0) {
    if (eflag == RunInfoBuffer::PROCESS_DOWN) {
      if (getNode().getState() == RCState::RUNNING_S) {
        abort();
        NSMCommunicator& com(*getCommunicator());
        if (load()) {
          start();
          getNode().setState(RCState::RUNNING_S);
          com.replyOK(getNode());
        } else {
          com.replyError(RunInfoBuffer::PROCESS_DOWN,
                         "Process recover failed " +
                         m_con[0].getExecutable());
        }
      }
    }
  }
}

