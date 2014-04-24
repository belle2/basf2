#include "daq/slc/hvcontrol/HVCallback.h"

#include <daq/slc/hvcontrol/HVCommand.h>
#include <daq/slc/hvcontrol/HVState.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

#include <iostream>

using namespace Belle2;

HVCallback::HVCallback(const NSMNode& node) throw()
  : NSMCallback(node)
{
  add(HVCommand::CONFIGURE);
  add(HVCommand::RAMPUP);
  add(HVCommand::RAMPDOWN);
  add(HVCommand::STANDBY);
  add(HVCommand::STANDBY2);
  add(HVCommand::STANDBY3);
  add(HVCommand::PEAK);
  add(HVCommand::TURNON);
  add(HVCommand::TURNOFF);
  getNode().setState(HVState::OFF_S);
  _state_demand = HVState::OFF_S;
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
  _state_demand = HVState::UNKNOWN;
  if (cmd == HVCommand::TURNOFF) {
    getNode().setState(HVState::TRANSITION_TS);
    result = turnoff();
  } else if (state.isOff()) {
    if (cmd == HVCommand::CONFIGURE) {
      result = config();
    } else if (cmd == HVCommand::TURNON) {
      getNode().setState(HVState::TRANSITION_TS);
      _state_demand = HVState::STANDBY_S;
      result = turnon();
    }
  } else if (state.isOn()) {
    if (cmd == HVCommand::RAMPUP) {
      if (state != HVState::PEAK_S) {
        getNode().setState(HVState::RAMPINGUP_TS);
        if (state == HVState::STANDBY_S) {
          _state_demand = HVState::STANDBY2_S;
        } else if (state == HVState::STANDBY2_S) {
          _state_demand = HVState::STANDBY3_S;
        } else if (state == HVState::STANDBY3_S) {
          _state_demand = HVState::PEAK_S;
        }
        result = rampup();
      }
    } else if (cmd == HVCommand::RAMPDOWN) {
      if (state != HVState::STANDBY_S) {
        getNode().setState(HVState::RAMPINGDOWN_TS);
        result = rampdown();
        if (state == HVState::STANDBY2_S) {
          _state_demand = HVState::STANDBY_S;
        } else if (state == HVState::STANDBY3_S) {
          _state_demand = HVState::STANDBY2_S;
        } else if (state == HVState::PEAK_S) {
          _state_demand = HVState::STANDBY3_S;
        }
      }
    } else if (cmd == HVCommand::PEAK) {
      getNode().setState(HVState::TRANSITION_TS);
      if (state != HVState::PEAK_S) {
        _state_demand = HVState::PEAK_S;
        result = peak();
      }
    } else if (cmd == HVCommand::STANDBY) {
      getNode().setState(HVState::TRANSITION_TS);
      if (state != HVState::STANDBY_S) {
        _state_demand = HVState::STANDBY_S;
        result = standby();
      }
    } else if (cmd == HVCommand::STANDBY2) {
      getNode().setState(HVState::TRANSITION_TS);
      if (state != HVState::STANDBY2_S) {
        _state_demand = HVState::STANDBY2_S;
        result = standby2();
      }
    } else if (cmd == HVCommand::STANDBY3) {
      getNode().setState(HVState::TRANSITION_TS);
      if (state != HVState::STANDBY3_S) {
        _state_demand = HVState::STANDBY3_S;
        result = standby3();
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
