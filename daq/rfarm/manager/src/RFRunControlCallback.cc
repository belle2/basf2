#include "daq/rfarm/manager/RFRunControlCallback.h"

#include "daq/rfarm/manager/RFMaster.h"
#include "daq/rfarm/manager/RFMasterCallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

using namespace Belle2;

RFRunControlCallback::RFRunControlCallback(const NSMNode& node,
                                           RFMaster* master,
                                           RFMasterCallback* callback)
  : RCCallback(node), _master(master), _callback(callback)
{
}

RFRunControlCallback::~RFRunControlCallback() throw()
{

}

bool RFRunControlCallback::load() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  _callback->getCommunicator()->getContext();
  b2nsm_context(nsmc);
  _master->Configure(msg, nsmc);
  return true;
}

bool RFRunControlCallback::start() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  _callback->getCommunicator()->getContext();
  b2nsm_context(nsmc);
  _master->Start(msg, nsmc);
  return true;
}

bool RFRunControlCallback::stop() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  _callback->getCommunicator()->getContext();
  b2nsm_context(nsmc);
  _master->Stop(msg, nsmc);
  return true;
}

bool RFRunControlCallback::recover() throw()
{
  NSMcontext* nsmc =  _callback->getCommunicator()->getContext();
  b2nsm_context(nsmc);
  return (abort() && load());
}

bool RFRunControlCallback::abort() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  _callback->getCommunicator()->getContext();
  b2nsm_context(nsmc);
  _master->UnConfigure(msg, nsmc);
  return true;
}

