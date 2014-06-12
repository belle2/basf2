#include "daq/slc/hvcontrol/HVControlCallback.h"

#include "daq/slc/hvcontrol/HVCommand.h"
#include "daq/slc/hvcontrol/hv_status.h"

#include <daq/slc/database/ConfigObjectTable.h>
#include <daq/slc/database/LoggerObjectTable.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>
#include <iostream>

using namespace Belle2;

HVControlCallback::HVControlCallback(const NSMNode& node)
throw() : HVCallback(node), m_db(NULL),
  m_data(node.getName() + "_STATUS",
         "hv_status", hv_status_revision)
{
}

HVControlCallback::~HVControlCallback() throw()
{
}

void HVControlCallback::init() throw()
{
  NSMCommunicator* comm = getCommunicator();
  hv_status* status = (hv_status*)m_data.allocate(comm);
  if (m_db != NULL) {
    m_db->connect();
    ConfigObjectTable table(m_db);
    std::string nodename = getNode().getName();
    ConfigObject obj(table.get("default", nodename));
    status->configid = obj.getId();
    m_config.set(obj);
    for (size_t i = 0; i < m_config.getNChannels(); i++) {
      m_status_v.push_back(HVChannelStatus());
    }
    m_db->close();
  }
  PThread(new HVNodeMonitor(this));
  initialize();
}

bool HVControlCallback::config() throw()
{
  std::string configname = getMessage().getData();
  if (configname.size() > 0 && m_db != NULL) {
    try {
      m_db->connect();
      ConfigObjectTable table(m_db);
      std::string nodename = getNode().getName();
      ConfigObject obj(table.get(configname, nodename));
      hv_status* status = (hv_status*)m_data.get();
      status->configid = obj.getId();
      m_config.set(obj);
    } catch (const DBHandlerException& e) {
      LogFile::debug("DB access error:%s", e.what());
      setReply("DB access error");
      m_db->close();
      return false;
    }
    m_db->close();
  }
  if (configure()) return true;
  return false;
}

void HVControlCallback::monitor() throw()
{
  static int count = 0;
  bool is_transition = false;
  bool is_error = false;
  bool is_off = true;
  hv_status* status = (hv_status*)m_data.get();
  for (size_t i = 0; i < m_config.getNChannels() && i < MAX_HVCHANNELS; i++) {
    HVChannelStatus& ch_status(m_status_v[i]);
    if (ch_status.getState() != HVState::OFF_S) is_off = false;
    if (ch_status.getState().isTransition()) is_transition = true;
    else if (ch_status.getState().isError()) is_error = true;
    ch_status.get(&status->channel[i]);
  }
  HVState s, s_org = getNode().getState();
  if (is_error) s = HVState::ERROR_ES;
  else if (is_transition) s = HVState::TRANSITION_TS;
  else if (is_off) s = HVState::OFF_S;
  else {
    s = getStateDemand();
    if (s == HVState::OFF_S && !is_off) {
      s = HVState::TRANSITION_TS;
    }
  }
  getNode().setState(s);
  status->state = s.getId();
  count++;
  if (!s_org.isError() && s.isError()) {
    getCommunicator()->replyError(getReply());
  } else if ((s_org.isTransition() && s.isStable()) ||
             (s_org != HVState::OFF_S && s == HVState::OFF_S) ||
             (count >= 3 && s != HVState::OFF_S)) {
    getCommunicator()->replyOK(getNode());
  }
  //m_db->connect();
  //LoggerObjectTable(m_db).add(m_data);
  //m_db->close();
  if (count >= 3) count = 0;
}

void HVControlCallback::HVNodeMonitor::run()
{
  while (true) {
    sleep(m_callback->getTimeout());
    m_callback->monitor();
  }
}
