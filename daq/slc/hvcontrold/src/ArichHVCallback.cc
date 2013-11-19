#include "ArichHVCallback.h"
#include "ArichHVMessage.h"

#include "HVCommand.h"
#include "HVState.h"

#include "nsm/NSMCommunicator.h"

#include <iostream>

using namespace Belle2;

ArichHVCallback::ArichHVCallback(NSMNode* node) throw()
  : HVCallback(node)
{
  download();
}

bool ArichHVCallback::load() throw()
{
  size_t nslot = getCrate()->getNSlot();
  size_t nchannel = getCrate()->getNChannel();
  std::cout << nslot << " " << nchannel << std::endl;
  for (size_t ns = 0; ns < nslot; ns++) {
    for (size_t nc = 0; nc < nchannel; nc++) {
      //getCrate()->getChannel(ns, nc)->print();
      std::cout << ArichHVMessage(ArichHVMessage::SET, ArichHVMessage::ALL,
                                  getCrate()->getChannel(ns, nc)).toString() << std::endl;
    }
  }
  return true;
}

bool ArichHVCallback::switchOn() throw()
{
  return true;
}

bool ArichHVCallback::switchOff() throw()
{
  return true;
}
