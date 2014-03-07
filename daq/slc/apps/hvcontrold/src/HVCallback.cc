#include "daq/slc/apps/hvcontrold/HVCallback.h"

#include "daq/slc/apps/hvcontrold/HVCommand.h"
#include "daq/slc/apps/hvcontrold/HVState.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/system/TCPSocketWriter.h"
#include "daq/slc/system/TCPSocketReader.h"
#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/ConfigFile.h"

#include <iostream>

using namespace Belle2;

HVCallback::HVCallback(NSMNode* node) throw()
  : NSMCallback(node)
{
  add(HVCommand::CONFIGURE);
  add(HVCommand::SAVE);
  add(HVCommand::RAMPUP);
  add(HVCommand::RAMPDOWN);
  add(HVCommand::STANDBY);
  add(HVCommand::STANDBY2);
  add(HVCommand::STANDBY3);
  add(HVCommand::PEAK);
  add(HVCommand::TURNON);
  add(HVCommand::TURNOFF);
  add(Command::STATECHECK);
  _node->setState(HVState::OFF_S);
}

bool HVCallback::perform(NSMMessage& msg)
throw(NSMHandlerException)
{
  HVCommand cmd(msg.getRequestName());
  LogFile::debug(cmd.getLabel());
  setReply("");
  if (cmd == Command::OK) {
    return ok();
  } else if (cmd == Command::ERROR) {
    return error();
  }
  HVState state(_node->getState());
  bool result = true;
  NSMCommunicator* com = getCommunicator();
  if (cmd == HVCommand::TURNOFF) {
    _node->setState(HVState::TRANSITION_TS);
    if ((result = turnoff())) {
      _node->setState(HVState::OFF_S);
    }
  } else if (state.isOff()) {
    if (cmd == HVCommand::CONFIGURE) {
      result = config();
    } else if (cmd == HVCommand::SAVE) {
      result = save();
    } else if (cmd == HVCommand::TURNON) {
      _node->setState(HVState::TRANSITION_TS);
      if ((result = turnon())) {
        _node->setState(HVState::STANDBY_S);
      }
    }
  } else if (state.isOn()) {
    if (cmd == HVCommand::RAMPUP) {
      if (state != HVState::PEAK_S) {
        _node->setState(HVState::RAMPINGUP_TS);
        if ((result = rampup())) {
          if (state == HVState::STANDBY_S) {
            _node->setState(HVState::STANDBY2_S);
          } else if (state == HVState::STANDBY2_S) {
            _node->setState(HVState::STANDBY3_S);
          } else if (state == HVState::STANDBY3_S) {
            _node->setState(HVState::PEAK_S);
          }
        }
      }
    } else if (cmd == HVCommand::RAMPDOWN) {
      if (state != HVState::STANDBY_S) {
        _node->setState(HVState::RAMPINGDOWN_TS);
        if ((result = rampdown())) {
          if (state == HVState::STANDBY2_S) {
            _node->setState(HVState::STANDBY_S);
          } else if (state == HVState::STANDBY3_S) {
            _node->setState(HVState::STANDBY2_S);
          } else if (state == HVState::PEAK_S) {
            _node->setState(HVState::STANDBY3_S);
          }
        }
      }
    } else if (cmd == HVCommand::PEAK) {
      _node->setState(HVState::TRANSITION_TS);
      if (state != HVState::PEAK_S) {
        if ((result = peak())) {
          _node->setState(HVState::PEAK_S);
        }
      }
    } else if (cmd == HVCommand::STANDBY) {
      _node->setState(HVState::TRANSITION_TS);
      if (state != HVState::STANDBY_S) {
        if ((result = standby())) {
          _node->setState(HVState::STANDBY_S);
        }
      }
    } else if (cmd == HVCommand::STANDBY2) {
      _node->setState(HVState::TRANSITION_TS);
      if (state != HVState::STANDBY2_S) {
        if ((result = standby2())) {
          _node->setState(HVState::STANDBY2_S);
        }
      }
    } else if (cmd == HVCommand::STANDBY3) {
      _node->setState(HVState::TRANSITION_TS);
      if (state != HVState::STANDBY3_S) {
        if ((result = standby3())) {
          _node->setState(HVState::STANDBY3_S);
        }
      }
    }
  } else if (cmd == Command::STATECHECK) {
    com->replyOK(_node, "");
    return true;
  }

  if (result) {
    com->replyOK(_node, _reply);
  } else {
    _node->setState(State::ERROR_ES);
    com->replyError(_reply);
  }
  LogFile::debug(_node->getState().getLabel());
  return result;
}
