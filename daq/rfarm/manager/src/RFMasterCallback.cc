#include "daq/rfarm/manager/RFMasterCallback.h"

#include "daq/rfarm/manager/RFMaster.h"
#include "daq/rfarm/manager/RFCommand.h"
#include "daq/rfarm/manager/RFNSM.h"

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/State.h>

#include <unistd.h>

using namespace Belle2;

RFMasterCallback::RFMasterCallback(NSMNode* node, NSMData* data, RFMaster* master)
  : NSMCallback(node), _data(data), _master(master)
{
}

RFMasterCallback::~RFMasterCallback() throw()
{

}

bool RFMasterCallback::perform(const Command& cmd, NSMMessage&)
throw(NSMHandlerException)
{
  setReply("");
  if (cmd == Command::OK) {
    return ok();
  } else if (cmd == Command::ERROR) {
    return error();
  }
  bool result = false;
  NSMCommunicator* com = getCommunicator();
  if (cmd == RFCommand::RF_CONFIGURE) {
    result = configure();
  } else if (cmd == RFCommand::RF_UNCONFIGURE) {
    result = unconfigure();
  } else if (cmd == RFCommand::RF_START) {
    result = start();
  } else if (cmd == RFCommand::RF_STOP) {
    result = stop();
  } else if (cmd == RFCommand::RF_RESTART) {
    result = restart();
  } else if (cmd == RFCommand::RF_PAUSE) {
    result = pause();
  } else if (cmd == RFCommand::RF_RESUME) {
    result = resume();
  } else if (cmd == RFCommand::RF_STATUS) {
    result = status();
  }
  if (result) {
    com->replyOK(_node, _reply);
    return true;
  } else {
    _node->setState(State::ERROR_ES);
    com->replyError(_reply);
  }
  return false;
}

void RFMasterCallback::init() throw()
{
  _master->SetNodeInfo((RfNodeInfo*)_data->get());
}

bool RFMasterCallback::ok() throw()
{
  RFNSM_Status& rfs = RFNSM_Status::Instance();
  int flag = rfs.get_flag();
  flag++;
  rfs.set_flag(flag);
  return true;
}

bool RFMasterCallback::error() throw()
{
  RFNSM_Status::Instance().set_flag(-1);
  return true;
}

bool RFMasterCallback::configure() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nmsc =  getCommunicator()->getContext();
  _master->Configure(msg, nmsc);
  return true;
}

bool RFMasterCallback::unconfigure() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nmsc =  getCommunicator()->getContext();
  _master->UnConfigure(msg, nmsc);
  return true;
}

bool RFMasterCallback::start() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nmsc =  getCommunicator()->getContext();
  _master->Start(msg, nmsc);
  return true;
}

bool RFMasterCallback::stop() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nmsc =  getCommunicator()->getContext();
  _master->Stop(msg, nmsc);
  return true;
}

bool RFMasterCallback::pause() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nmsc =  getCommunicator()->getContext();
  _master->Pause(msg, nmsc);
  return true;
}

bool RFMasterCallback::resume() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nmsc =  getCommunicator()->getContext();
  _master->Resume(msg, nmsc);
  return true;
}

bool RFMasterCallback::restart() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nmsc =  getCommunicator()->getContext();
  _master->Restart(msg, nmsc);
  return true;
}

bool RFMasterCallback::status() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nmsc =  getCommunicator()->getContext();
  _master->Status(msg, nmsc);
  return true;
}


