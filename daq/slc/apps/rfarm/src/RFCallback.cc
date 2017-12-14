#include "daq/slc/apps/rfarm/RFCallback.h"

#include "daq/rfarm/manager/RFCommand.h"
#include "daq/rfarm/manager/RfNodeInfo.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

using namespace Belle2;

std::string state = "Unconfigured";

RFCallback::RFCallback(int timeout) throw()
  : NSMCallback(timeout)
{
  reg(RFCommand::CONFIGURE);
  reg(RFCommand::UNCONFIGURE);
  reg(RFCommand::START);
  reg(RFCommand::STOP);
  reg(RFCommand::RESUME);
  reg(RFCommand::PAUSE);
  reg(RFCommand::RESTART);
  reg(RFCommand::STATUS);
}

void RFCallback::init(NSMCommunicator& /*com*/) throw()
{

}

void RFCallback::configure() throw()
{
  //sleep(2);
  LogFile::info("configured");
  state = "Configured";
  reply(NSMMessage(NSMCommand::OK, state));
}

void RFCallback::unconfigure() throw()
{
  //sleep(2);
  LogFile::info("unconfigured");
  state = "Unconfigured";
  reply(NSMMessage(NSMCommand::OK, state));
}

void RFCallback::start() throw()
{
  LogFile::info("started");
  state = "Running";
  reply(NSMMessage(NSMCommand::OK, state));
}

void RFCallback::restart() throw()
{
  LogFile::info("restarted");
  state = "Running";
  reply(NSMMessage(NSMCommand::OK, state));
}

void RFCallback::stop() throw()
{
  LogFile::info("stopped");
  state = "Configured";
  reply(NSMMessage(NSMCommand::OK, state));
}

void RFCallback::resume() throw()
{
  LogFile::info("resumed");
  state = "Running";
  reply(NSMMessage(NSMCommand::OK, state));
}

void RFCallback::pause() throw()
{
  LogFile::info("paused");
  state = "Paused";
  reply(NSMMessage(NSMCommand::OK, state));
}

void RFCallback::status() throw()
{
  LogFile::info("status");
  reply(NSMMessage(NSMCommand::OK, state));
}

bool RFCallback::perform(NSMCommunicator& com) throw()
{
  const NSMMessage msg = com.getMessage();
  if (NSMCallback::perform(com)) {
    return true;
  }
  const RFCommand cmd = msg.getRequestName();
  LogFile::debug("%s >> %s", msg.getNodeName(), cmd.getLabel());
  addNode(NSMNode(msg.getNodeName()));
  try {
    if (cmd == RFCommand::CONFIGURE) {
      configure();
      return true;
    } else if (cmd == RFCommand::UNCONFIGURE) {
      unconfigure();
      return true;
    } else if (cmd == RFCommand::START) {
      start();
      return true;
    } else if (cmd == RFCommand::STOP) {
      stop();
      return true;
    } else if (cmd == RFCommand::RESTART) {
      restart();
      return true;
    } else if (cmd == RFCommand::PAUSE) {
      pause();
      return true;
    } else if (cmd == RFCommand::RESUME) {
      resume();
      return true;
    } else if (cmd == RFCommand::STATUS) {
      status();
      return true;
    }
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
  return false;
}
