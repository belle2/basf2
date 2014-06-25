#include "daq/rfarm/manager/RFMasterCallback.h"
#include "daq/rfarm/manager/RFRunControlCallback.h"

#include "daq/rfarm/manager/RFMaster.h"
#include "daq/rfarm/manager/RFCommand.h"
#include "daq/rfarm/manager/RFNSM.h"

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RfNodeInfo.h"
#include "daq/rfarm/manager/RfUnitInfo.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

using namespace Belle2;

RFMasterCallback::RFMasterCallback(const NSMNode& node,
                                   const NSMData& data,
                                   RFMaster* master)
  : NSMCallback(node), m_data(data), m_master(master)
{
}

RFMasterCallback::~RFMasterCallback() throw()
{

}

bool RFMasterCallback::perform(const NSMMessage& msg) throw()
{
  RFCommand cmd(msg.getRequestName());
  setReply("");
  if (cmd == NSMCommand::OK) {
    return ok();
  } else if (cmd == NSMCommand::ERROR) {
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
    com->replyOK(getNode());
    return true;
  } else {
    //getNode().setState(NSMState::ERROR_ES);
    com->replyError(getReply());
  }
  return false;
}

void RFMasterCallback::init() throw()
{
  m_master->SetNodeInfo((RfNodeInfo*)m_data.allocate(getCommunicator()));
  char* conffile = getenv("RFARM_CONFFILE");
  RFConf conf(conffile);
  char* format = conf.getconf("system", "nsmdata");
  char* node = conf.getconf("distributor", "nodename");
  m_data_v.push_back(NSMData(node, format, 1));
  node = conf.getconf("collector", "nodename");
  m_data_v.push_back(NSMData(node, format, 1));
  int maxnodes = conf.getconfi("processor", "nnodes");
  int idbase = conf.getconfi("processor", "idbase");
  char* hostbase = conf.getconf("processor", "nodebase");
  char* badlist = conf.getconf("processor", "badlist");
  char hostnode[512], idname[3];
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    if (badlist == NULL  ||
        strstr(badlist, idname) == 0) {
      sprintf(hostnode, "evp_%s%2.2d", hostbase, idbase + i);
      m_data_v.push_back(NSMData(hostnode, format, 1));
    }
  }
}

void RFMasterCallback::timeout() throw()
{
  if (!m_callback->getData().isAvailable()) return;
  RfUnitInfo* unitinfo = (RfUnitInfo*)m_callback->getData().get();
  unitinfo->nnodes = m_data_v.size();
  unitinfo->updatetime = Time().get();
  for (size_t i = 0; i < m_data_v.size(); i++) {
    if (!m_data_v[i].isAvailable()) {
      m_data_v[i].open(getCommunicator());
    }
    if (m_data_v[i].isAvailable()) {
      RfUnitInfo::RfNodeInfo* nodeinfo_o = &unitinfo->nodeinfo[i];
      RfNodeInfo* nodeinfo_i = (RfNodeInfo*)m_data_v[i].get();
      memcpy(nodeinfo_o, nodeinfo_i, sizeof(RfNodeInfo));
    }
  }
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
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Configure(msg, nsmc);
  return true;
}

bool RFMasterCallback::unconfigure() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->UnConfigure(msg, nsmc);
  return true;
}

bool RFMasterCallback::start() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Start(msg, nsmc);
  return true;
}

bool RFMasterCallback::stop() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Stop(msg, nsmc);
  return true;
}

bool RFMasterCallback::pause() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Pause(msg, nsmc);
  return true;
}

bool RFMasterCallback::resume() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Resume(msg, nsmc);
  return true;
}

bool RFMasterCallback::restart() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Restart(msg, nsmc);
  return true;
}

bool RFMasterCallback::status() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Status(msg, nsmc);
  return true;
}


