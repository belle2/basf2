#include "daq/slc/hvcontrol/HVCallback.h"

#include <daq/slc/hvcontrol/HVCommand.h>
#include <daq/slc/hvcontrol/HVState.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <iostream>

using namespace Belle2;

HVCallback::HVCallback(const NSMNode& node) throw()
  : NSMCallback(node)
{
  add(HVCommand::CONFIGURE);
  add(HVCommand::STANDBY);
  add(HVCommand::SHOULDER);
  add(HVCommand::PEAK);
  add(HVCommand::TURNON);
  add(HVCommand::TURNOFF);
  add(HVCommand::HVAPPLY);
  getNode().setState(HVState::OFF_S);
  m_state_demand = HVState::OFF_S;
}

bool HVCallback::perform(const NSMMessage& msg) throw()
{
  HVCommand cmd(msg.getRequestName());
  LogFile::debug(cmd.getLabel());
  setReply("");
  if (NSMCallback::perform(msg)) return true;
  HVState state(getNode().getState());
  bool result = true;
  NSMCommunicator* com = getCommunicator();

  if (cmd == HVCommand::HVAPPLY) {
    LogFile::debug("HVAPPLY : len=%d", msg.getLength());
    if (msg.getLength() == 0) {
      LogFile::warning("HVAPPLY without message");
      return true;
    }
    StringList str = StringUtil::split(msg.getData(), '=', 2);

    HVApplyMessage hvmsg(msg.getParam(0), msg.getParam(1),
                         msg.getParam(1), str[0], str[1]);
    result = hvapply(hvmsg);
  } else if (cmd == HVCommand::TURNOFF) {
    getNode().setState(HVState::TRANSITION_TS);
    result = turnoff();
  } else if (state.isOff()) {
    if (cmd == HVCommand::CONFIGURE) {
      result = config();
    } else if (cmd == HVCommand::TURNON) {
      getNode().setState(HVState::TRANSITION_TS);
      m_state_demand = HVState::STANDBY_S;
      result = turnon();
    }
  } else if (state.isOn()) {
    if (cmd == HVCommand::STANDBY) {
      if (state != HVState::STANDBY_S) {
        getNode().setState(HVState::TRANSITION_TS);
        m_state_demand = HVState::STANDBY_S;
        result = standby();
      }
    } else if (cmd == HVCommand::SHOULDER) {
      if (state != HVState::SHOULDER_S) {
        getNode().setState(HVState::TRANSITION_TS);
        m_state_demand = HVState::SHOULDER_S;
        result = shoulder();
      }
    } else if (cmd == HVCommand::PEAK) {
      if (state != HVState::PEAK_S) {
        getNode().setState(HVState::TRANSITION_TS);
        m_state_demand = HVState::PEAK_S;
        result = peak();
      }
    }
  }
  if (result) {
    com->replyOK(getNode());
  } else {
    getNode().setState(HVState::ERROR_ES);
    com->replyError(getReply());
  }
  LogFile::debug(getNode().getState().getLabel());
  return result;
}

