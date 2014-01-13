#include "daq/slc/nsm/NSMCallback.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include <signal.h>
#include <string.h>

using namespace Belle2;

void NSMCallback::terminate(int sig)
{
  LogFile::debug("%s: terminate process", strsignal(sig));
  for (size_t i = 0; i < __callback_v.size(); i++) {
    __callback_v[i]->term();
  }
  exit(1);
}

std::vector<NSMCallback*>  NSMCallback::__callback_v;

NSMCallback::NSMCallback(NSMNode* node) throw()
  : _node(node), _comm(NULL)
{
  add(Command::OK);
  add(Command::ERROR);
  add(Command::FATAL);
  signal(SIGINT, terminate);
  signal(SIGQUIT, terminate);
  signal(SIGTERM, terminate);
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
