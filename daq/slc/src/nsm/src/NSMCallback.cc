#include "nsm/NSMCallback.h"

#include "nsm/NSMCommunicator.h"
#include "nsm/NSMMessage.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

using namespace Belle2;

NSMCallback::NSMCallback(NSMNode* node) throw()
  : _node(node), _comm(NULL)
{
  add(Command::OK);
  add(Command::ERROR);
}

bool NSMCallback::isReady() const throw()
{
  return _comm != NULL && _comm->isOnline();
}

bool NSMCallback::perform(NSMMessage& msg) throw(NSMHandlerException)
{
  std::string cmd_s(msg.getRequestName());
  if (cmd_s == Command::OK.getLabel()) {
    return ok();
  } else if (cmd_s == Command::ERROR.getLabel()) {
    return error();
  }
  throw (NSMHandlerException(__FILE__, __LINE__, "Wrong request"));
}

NSMMessage& NSMCallback::getMessage()
{
  return _comm->getMessage();
}

int NSMCallback::getExpNumber()
{
  if (getMessage().getNParams() > 1) {
    return (int)getMessage().getParam(0);
  } else {
    return -1;
  }
}

int NSMCallback::getRunNumber()
{
  if (getMessage().getNParams() > 1) {
    return (int)getMessage().getParam(1);
  } else {
    return -1;
  }
}
