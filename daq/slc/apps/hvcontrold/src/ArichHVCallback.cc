#include "daq/slc/apps/hvcontrold/ArichHVCallback.h"

#include "daq/slc/apps/hvcontrold/ArichHVMessage.h"
#include "daq/slc/apps/hvcontrold/ArichHVCommunicator.h"

#include "daq/slc/apps/hvcontrold/HVCommand.h"
#include "daq/slc/apps/hvcontrold/HVState.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include <iostream>

using namespace Belle2;

ArichHVCallback::ArichHVCallback(NSMNode* node) throw()
  : HVCallback(node)
{
}

bool ArichHVCallback::load() throw()
{
  size_t nslot = getCrate()->getNSlot();
  size_t nchannel = getCrate()->getNChannel();
  try {
    for (size_t ns = 0; ns < nslot; ns++) {
      for (size_t nc = 0; nc < nchannel; nc++) {
        HVChannelInfo* info = getCrate()->getChannelInfo(ns, nc);
        HVChannelStatus* status = getCrate()->getChannelStatus(ns, nc);
        ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::ALL,
                           info, status);
        msg.setParamType(ArichHVMessage::VOLTAGE_DEMAND);
        _hv_comm->sendRequest(msg);
        msg.setParamType(ArichHVMessage::VOLTAGE_LIMIT);
        _hv_comm->sendRequest(msg);
        msg.setParamType(ArichHVMessage::CURRENT_LIMIT);
        _hv_comm->sendRequest(msg);
        msg.setParamType(ArichHVMessage::RAMPUP_SPEED);
        _hv_comm->sendRequest(msg);
        msg.setParamType(ArichHVMessage::RAMPDOWN_SPEED);
        _hv_comm->sendRequest(msg);
      }
    }
  } catch (const IOException& e) {
    setReply("Socket IO error to HV crate");
    return false;
  }
  return true;
}

bool ArichHVCallback::switchOn() throw()
{
  size_t nslot = getCrate()->getNSlot();
  size_t nchannel = getCrate()->getNChannel();
  try {
    for (size_t ns = 0; ns < nslot; ns++) {
      for (size_t nc = 0; nc < nchannel; nc++) {
        HVChannelInfo* info = getCrate()->getChannelInfo(ns, nc);
        HVChannelStatus* status = getCrate()->getChannelStatus(ns, nc);
        ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::SWITCH,
                           info, status);
        _hv_comm->sendRequest(msg);
      }
    }
  } catch (const IOException& e) {
    setReply("Socket IO error to HV crate");
    return false;
  }
  return true;
}

bool ArichHVCallback::switchOff() throw()
{
  size_t nslot = getCrate()->getNSlot();
  size_t nchannel = getCrate()->getNChannel();
  try {
    for (size_t ns = 0; ns < nslot; ns++) {
      for (size_t nc = 0; nc < nchannel; nc++) {
        HVChannelInfo* info = getCrate()->getChannelInfo(ns, nc);
        HVChannelStatus* status = getCrate()->getChannelStatus(ns, nc);
        bool is_switch_on = info->isSwitchOn();
        info->setSwitchOn(false);
        ArichHVMessage msg(ArichHVMessage::SET,
                           ArichHVMessage::SWITCH,
                           info, status);
        _hv_comm->sendRequest(msg);
        info->setSwitchOn(is_switch_on);
      }
    }
  } catch (const IOException& e) {
    setReply("Socket IO error to HV crate");
    return false;
  }
  return true;
}
