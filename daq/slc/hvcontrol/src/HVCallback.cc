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
}

bool HVCallback::perform(NSMCommunicator& com) throw()
{
  if (NSMCallback::perform(com)) return true;
  lock();
  try {
    NSMMessage msg(com.getMessage());
    HVCommand cmd(msg.getRequestName());
    HVState state(getNode().getState());
    if (cmd == HVCommand::TURNOFF) {
      getNode().setState(HVState::TRANSITION_TS);
      turnoff();
    } else if (state.isOff()) {
      if (cmd == HVCommand::CONFIGURE) {
        m_confignames = msg.getData();
        dbload(m_confignames);
        addAll(getConfig());
        configure(getConfig());
      } else if (cmd == HVCommand::TURNON) {
        getNode().setState(HVState::TRANSITION_TS);
        m_state_demand = HVState::STANDBY_S;
        turnon();
      }
    } else if (state.isOn()) {
      if (cmd == HVCommand::STANDBY) {
        if (state != HVState::STANDBY_S) {
          getNode().setState(HVState::TRANSITION_TS);
          m_state_demand = HVState::STANDBY_S;
          standby();
        }
      } else if (cmd == HVCommand::SHOULDER) {
        if (state != HVState::SHOULDER_S) {
          getNode().setState(HVState::TRANSITION_TS);
          m_state_demand = HVState::SHOULDER_S;
          shoulder();
        }
      } else if (cmd == HVCommand::PEAK) {
        if (state != HVState::PEAK_S) {
          getNode().setState(HVState::TRANSITION_TS);
          m_state_demand = HVState::PEAK_S;
          peak();
        }
      }
    }
  } catch (const HVHandlerException& e) {
    reply(NSMMessage(NSMCommand::ERROR, e.what()));
  }
  unlock();
  return true;
}

void HVCallback::addAll(const HVConfig& config) throw()
{
  const HVCrateList& crate_v(config.getCrates());
  reset();
  add(new NSMVHandlerText("configs", true, false, m_confignames));
  add(new NSMVHandlerInt("ncrates", true, false, (int)crate_v.size()));
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
      vname = StringUtil::form("crate[%d].slot[%d].channel[%d]", crateid, slot, ch);
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

