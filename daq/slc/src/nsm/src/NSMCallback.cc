#include "NSMCallback.hh"

#include "NSMCommunicator.hh"
#include "NSMMessage.hh"

#include <cstdarg>
#include <cstdio>
#include <cstring>

using namespace B2DAQ;

NSMCallback::NSMCallback() throw()
{
  add(Command::OK);
  add(Command::ERROR);
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
