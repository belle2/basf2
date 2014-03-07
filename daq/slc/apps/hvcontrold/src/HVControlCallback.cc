#include "daq/slc/apps/hvcontrold/HVControlCallback.h"

#include "daq/slc/apps/hvcontrold/HVCommand.h"

#include <daq/slc/system/Time.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>

#include <sstream>
#include <iostream>
#include <cstring>
#include <unistd.h>

using namespace Belle2;

HVControlCallback::HVControlCallback(NSMNode* node)
throw() : HVCallback(node), m_status(NULL), m_data(NULL)
{
  m_info.setNode(node);
}

HVControlCallback::~HVControlCallback() throw()
{
  if (m_data != NULL) delete m_data;
}

void HVControlCallback::init() throw()
{
  NSMCommunicator* comm = getCommunicator();
  m_data = new NSMData(_node->getName() + "_STATUS",
                       "hv_status", hv_status_revision);
  m_status = (hv_status*)m_data->allocate(comm);
  memset(m_status, 0, sizeof(hv_status));
  if (m_db != NULL) {
    m_db->connect();
    if (m_info.create(m_db) == 0) {
      m_info.save(m_db);
    }
    m_db->close();
  }
  PThread(new HVNodeMonitor(this));
  initialize();
}

bool HVControlCallback::rampup() throw()
{
  HVState state(_node->getState());
  if (state == HVState::STANDBY_S) {
    return standby2();
  } else if (state == HVState::STANDBY2_S) {
    return standby3();
  } else if (state == HVState::STANDBY3_S) {
    return peak();
  }
  return true;
}

bool HVControlCallback::rampdown() throw()
{
  HVState state(_node->getState());
  if (state == HVState::STANDBY2_S) {
    return standby();
  } else if (state == HVState::STANDBY3_S) {
    return standby2();
  } else if (state == HVState::PEAK_S) {
    return standby3();
  }
  return true;
}

bool HVControlCallback::save() throw()
{
  if (m_db != NULL) {
    m_db->connect();
    m_info.save(m_db);
    m_db->close();
  }
  setReply(Belle2::form("%d", m_info.getConfigId()));
  return true;
}

bool HVControlCallback::config() throw()
{
  m_info.setConfigId(getMessage().getParam(0));
  LogFile::debug("config id = %d", m_info.getConfigId());
  if (m_db != NULL) {
    try {
      m_db->connect();
      m_info.load(m_db);
    } catch (const DBHandlerException& e) {
      LogFile::debug("DB access error:%s", e.what());
      setReply("DB access error");
      m_db->close();
      return false;
    }
    m_db->close();
  }
  if (configure()) {
    setReply(Belle2::form("%d", m_info.getConfigId()));
    return true;
  }
  return false;
}

void HVControlCallback::HVNodeMonitor::run()
{
  while (true) {
    sleep(m_callback->getInterval());
    //monitor();
    bool is_transition = false;
    bool is_error = false;
    bool is_peak = true;
    bool is_off = true;
    hv_status* status = m_callback->m_status;
    const size_t max_ch = sizeof(status->ch_state) / sizeof(status->ch_state[0]);
    status->state = m_callback->getNode()->getState().getId();
    HVChannelInfoList& ch_info_v(m_callback->m_info.getChannelList());
    for (size_t i = 0; i < ch_info_v.size() && i < max_ch; i++) {
      hv_channel_status& ch_status(ch_info_v[i].getStatus().getStatus());
      HVState ch_state = ch_info_v[i].getStatus().getState();
      if (ch_state != HVState::PEAK_S) is_peak = false;
      if (ch_state != HVState::OFF_S) is_off = false;
      if (ch_state.isTransition()) is_transition = true;
      else if (ch_state.isError()) is_error = true;
      status->ch_state[i] = ch_status.state;
      status->voltage_mon[i] = ch_status.voltage_mon;
      status->current_mon[i] = ch_status.current_mon;
      LogFile::debug("index=%d, state=%s, voltage=%f, current=%f", i,
                     ch_state.getLabel(),
                     status->voltage_mon[i], status->current_mon[i]);
    }
    if (is_error) status->state = HVState::ERROR_ES.getId();
    else if (is_transition) status->state = HVState::TRANSITION_TS.getId();
    else if (is_peak) status->state = HVState::PEAK_S.getId();
    else if (is_off) status->state = HVState::OFF_S.getId();
  }
}
