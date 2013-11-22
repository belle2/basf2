#include "ArichHVCallback.h"

#include "ArichHVMessage.h"
#include "ArichHVCommunicator.h"

#include "HVCommand.h"
#include "HVState.h"

#include "nsm/NSMCommunicator.h"

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
        ArichHVMessage msg(ArichHVMessage::SET,
                           ArichHVMessage::ALL,
                           getCrate()->getChannel(ns, nc));
        std::cout << msg.toString() << std::endl;
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
        ArichHVMessage msg(ArichHVMessage::SET,
                           ArichHVMessage::SWITCH,
                           getCrate()->getChannel(ns, nc));
        std::cout << msg.toString() << std::endl;
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
        HVChannelInfo* info = getCrate()->getChannel(ns, nc);
        bool is_switch_on = info->isSwitchOn();
        info->setSwitchOn(false);
        ArichHVMessage msg(ArichHVMessage::SET,
                           ArichHVMessage::SWITCH,
                           info);
        std::cout << msg.toString() << std::endl;
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
