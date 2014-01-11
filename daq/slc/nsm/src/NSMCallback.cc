#include "daq/slc/nsm/NSMCallback.h"

#include "daq/slc/nsm/NSMCommunicator.h"

using namespace Belle2;

NSMCallback::NSMCallback(NSMNode* node) throw()
  : _node(node), _comm(NULL)
{
  add(Command::OK);
  add(Command::ERROR);
  add(Command::FATAL);
}

bool NSMCallback::isReady() const throw()
{
  return _comm != NULL && _comm->isOnline();
}

bool NSMCallback::perform(const Command& cmd, NSMMessage&)
throw(NSMHandlerException)
{
  if (cmd == Command::OK) {
    return ok();
  } else if (cmd == Command::ERROR) {
    return error();
  }
  throw (NSMHandlerException(__FILE__, __LINE__, "Wrong request"));
}

NSMMessage& NSMCallback::getMessage()
{
  return _msg;
}

void NSMCallback::setMessage(NSMMessage& msg)
{
  _msg = msg;
  if (_comm != NULL) _comm->setMessage(msg);
}

int NSMCallback::getExpNumber()
{
  if (getMessage().getNParams() > 1) {
    return (int)getMessage().getParam(0);
  } else {
    return -1;
  }
}

int NSMCallback::getColdNumber()
{
  if (getMessage().getNParams() > 1) {
    return (int)getMessage().getParam(1);
  } else {
    return -1;
  }
}

int NSMCallback::getHotNumber()
{
  if (getMessage().getNParams() > 2) {
    return (int)getMessage().getParam(2);
  } else {
    return -1;
  }
}
