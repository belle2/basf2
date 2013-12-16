#include "daq/rfarm/manager/RFRunControlCallback.h"

#include "daq/rfarm/manager/RFMaster.h"
#include "daq/rfarm/manager/RFMasterCallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/State.h>

#include <unistd.h>

using namespace Belle2;

RFRunControlCallback::RFRunControlCallback(NSMNode* node, RFMaster* master,
                                           RFMasterCallback* callback)
  : RCCallback(node), _master(master), _callback(callback)
{
  node->setData(new DataObject());
  node->setState(State::INITIAL_S);
}

RFRunControlCallback::~RFRunControlCallback() throw()
{

}

bool RFRunControlCallback::boot() throw()
{
  Belle2::debug("BOOT");
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  _callback->getCommunicator()->getContext();
  b2nsm_context(nsmc);
  _master->Configure(msg, nsmc);
  return true;
}

bool RFRunControlCallback::load() throw()
{
  Belle2::debug("LOAD");
  NSMcontext* nsmc =  _callback->getCommunicator()->getContext();
  b2nsm_context(nsmc);
  return true;
}

bool RFRunControlCallback::start() throw()
{
  Belle2::debug("START");
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  _callback->getCommunicator()->getContext();
  b2nsm_context(nsmc);
  _master->Start(msg, nsmc);
  return true;
}

bool RFRunControlCallback::stop() throw()
{
  Belle2::debug("STOP");
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  _callback->getCommunicator()->getContext();
  b2nsm_context(nsmc);
  _master->Stop(msg, nsmc);
  return true;
}

bool RFRunControlCallback::recover() throw()
{
  Belle2::debug("RECOVER");
  NSMcontext* nsmc =  _callback->getCommunicator()->getContext();
  b2nsm_context(nsmc);
  return true;
}

