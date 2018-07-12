#include "daq/slc/hvcontrol/HVCallback.h"

#include "daq/slc/hvcontrol/HVVHandler.h"
#include "daq/slc/hvcontrol/HVCommand.h"
#include "daq/slc/hvcontrol/HVState.h"
#include "daq/slc/hvcontrol/HVMessage.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <iostream>

using namespace Belle2;

HVCallback::HVCallback() throw() : NSMCallback()
{
  reg(HVCommand::CONFIGURE);
  reg(HVCommand::STANDBY);
  reg(HVCommand::SHOULDER);
  reg(HVCommand::PEAK);
  reg(HVCommand::TURNON);
  reg(HVCommand::TURNOFF);
  m_state_demand = HVState::OFF_S;
  m_config = FLAG_STANDBY;
}

HVConfig& HVCallback::getConfig() throw()
{
  switch (m_config) {
    case FLAG_STANDBY:
      return m_config_standby;
    case FLAG_SHOULDER:
      return m_config_shoulder;
    case FLAG_PEAK:
      return m_config_peak;
  }
  return m_config_standby;
}

const HVConfig& HVCallback::getConfig() const throw()
{
  switch (m_config) {
    case FLAG_STANDBY:
      return m_config_standby;
    case FLAG_SHOULDER:
      return m_config_shoulder;
    case FLAG_PEAK:
      return m_config_peak;
  }
  return m_config_standby;
}

bool HVCallback::perform(NSMCommunicator& com) throw()
{
  NSMMessage msg(com.getMessage());
  if (NSMCallback::perform(com)) {
    return true;
  }
  try {
    HVCommand cmd(msg.getRequestName());
    if (cmd == HVCommand::UNKNOWN) return false;
    HVState state(getNode().getState());
    HVState tstate(cmd.nextTState(state));
    addNode(NSMNode(msg.getNodeName()));
    if (cmd == HVCommand::TURNOFF) {
      m_state_demand = HVState::OFF_S;
      setHVState(HVState::TURNINGOFF_TS);
      m_config = FLAG_STANDBY;
      turnoff();
    } else if (state.isOff()) {
      if (cmd == HVCommand::CONFIGURE) {
        const char* data = msg.getData();
        if (msg.getLength() && strlen(data) > 0) {
          StringList names = StringUtil::split(data, ',');
          for (size_t i = 0; i < names.size(); i++) {
            StringList s = StringUtil::split(names[i], '=');
            if (s.size() > 1) {
              if (s[0] == "standby") {
                m_configname_standby = s[1];
              } else if (s[0] == "peak") {
                m_configname_peak = s[1];
              }
            }
          }
        } else {
          LogFile::warning("No config name");
        }
        LogFile::info("standby  : %s", m_configname_standby.c_str());
        LogFile::info("peak     : %s", m_configname_peak.c_str());
        reset();
        //lock();
        dbload(m_config_standby, m_configname_standby);
        dbload(m_config_peak, m_configname_peak);
        addAll(m_config_standby);
        set("config.standby", m_configname_standby);
        set("config.peak", m_configname_peak);
        m_config = FLAG_STANDBY;
        addAll(getConfig());
        configure(getConfig());
        //unlock();
      } else if (cmd == HVCommand::TURNON) {
        get("config.standby", m_configname_standby);
        //lock();
        //dbload(m_config_standby, m_configname_standby);
        //addAll(m_config_standby);
        //unlock();
        setHVState(tstate);
        m_state_demand = HVState::STANDBY_S;
        m_config = FLAG_STANDBY;
        turnon();
      }
    } else if (state.isOn()) {
      if (cmd == HVCommand::STANDBY && state != HVState::STANDBY_S) {
        get("config.standby", m_configname_standby);
        dbload(m_config_standby, m_configname_standby);
        addAll(m_config_standby);
        setHVState(tstate);
        m_state_demand = HVState::STANDBY_S;
        m_config = FLAG_STANDBY;
        //lock();
        //configure(getConfig());
        //unlock();
        standby();
      } else if (cmd == HVCommand::PEAK && state != HVState::PEAK_S) {
        get("config.peak", m_configname_peak);
        dbload(m_config_peak, m_configname_peak);
        addAll(m_config_peak);
        setHVState(tstate);
        m_state_demand = HVState::PEAK_S;
        m_config = FLAG_PEAK;
        //lock();
        //configure(getConfig());
        //unlock();
        peak();
      }
    }
  } catch (const HVHandlerException& e) {
    reply(NSMMessage(NSMCommand::ERROR, e.what()));
  }
  return true;
}

void HVCallback::addAll(const HVConfig& config) throw()
{
  const HVCrateList& crate_v(config.getCrates());
  reset();
  add(new NSMVHandlerText("config", true, false, config.getName()));
  add(new NSMVHandlerText("config.standby", true, true, m_configname_standby));
  add(new NSMVHandlerText("config.peak", true, true, m_configname_peak));
  add(new NSMVHandlerText("hvstate", true, false, getNode().getState().getLabel()));
  add(new NSMVHandlerInt("ncrates", true, false, (int)crate_v.size()));
  int l = 0;
  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    const HVChannelList& channel_v(crate.getChannels());
    int crateid = crate.getId();
    std::string vname = StringUtil::form("crate[%d]", crateid);
    add(new NSMVHandlerText(vname + ".name", true, false, crate.getName()));
    add(new NSMVHandlerInt(vname + ".nchannels", true, false, (int)channel_v.size()));
    add(new NSMVHandlerText(vname + ".host", true, false, crate.getHost()));
    add(new NSMVHandlerInt(vname + ".port", true, false, crate.getPort()));
    for (size_t i = 0; i < channel_v.size(); i++) {
      const HVChannel& channel(channel_v[i]);
      int slot = channel.getSlot();
      int ch = channel.getChannel();
      m_mon_tmp[crateid][l].state = -1;
      m_mon_tmp[crateid][l].vmon = -1;
      m_mon_tmp[crateid][l].cmon = -1;
      l++;
      /*
      std::string vname = StringUtil::form("crate[%d].channel[%d]", crateid, i);
      add(new NSMVHandlerInt(vname + ".channel", true, false, ch));
      add(new NSMVHandlerInt(vname + ".slot", true, false, slot));
      add(new NSMVHandlerHVSwitch(*this, vname + ".switch", crateid, slot, ch));
      add(new NSMVHandlerHVRampUpSpeed(*this, vname + ".rampup", crateid, slot, ch));
      add(new NSMVHandlerHVRampDownSpeed(*this, vname + ".rampdown", crateid, slot, ch));
      add(new NSMVHandlerHVVoltageDemand(*this, vname + ".vdemand", crateid, slot, ch));
      add(new NSMVHandlerHVVoltageLimit(*this, vname + ".vlimit", crateid, slot, ch));
      add(new NSMVHandlerHVCurrentLimit(*this, vname + ".climit", crateid, slot, ch));
      add(new NSMVHandlerHVState(*this, vname + ".state", crateid, slot, ch));
      add(new NSMVHandlerHVVoltageMonitor(*this, vname + ".vmon", crateid, slot, ch));
      add(new NSMVHandlerHVCurrentMonitor(*this, vname + ".cmon", crateid, slot, ch));
      */
      std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d]", crateid, slot, ch);
      add(new NSMVHandlerInt(vname + ".channel", true, false, ch));
      add(new NSMVHandlerInt(vname + ".slot", true, false, slot));
      add(new NSMVHandlerHVSwitch(*this, vname + ".switch", crateid, slot, ch));
      add(new NSMVHandlerHVRampUpSpeed(*this, vname + ".rampup", crateid, slot, ch));
      add(new NSMVHandlerHVRampDownSpeed(*this, vname + ".rampdown", crateid, slot, ch));
      add(new NSMVHandlerHVVoltageDemand(*this, vname + ".vdemand", crateid, slot, ch));
      add(new NSMVHandlerHVVoltageLimit(*this, vname + ".vlimit", crateid, slot, ch));
      add(new NSMVHandlerHVCurrentLimit(*this, vname + ".climit", crateid, slot, ch));
      add(new NSMVHandlerHVState(*this, vname + ".state", crateid, slot, ch));
      add(new NSMVHandlerHVVoltageMonitor(*this, vname + ".vmon", crateid, slot, ch));
      add(new NSMVHandlerHVCurrentMonitor(*this, vname + ".cmon", crateid, slot, ch));
    }
  }
}

void HVCallback::setHVState(const HVState& state)
{
  getNode().setState(state);
  set("hvstate", state.getLabel());
  log(LogFile::INFO, "HV State >> %s", state.getLabel());
}

