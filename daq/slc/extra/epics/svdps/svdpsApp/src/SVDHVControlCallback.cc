#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"

#include "SVDHVControlCallback.h"

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void printChidInfo(chid chid, const char* message);
void connectionCallback(struct connection_handler_args args);
void accessRightsCallback(struct access_rights_handler_args args);
void eventCallback(struct event_handler_args eha);

SVDHVControlCallback::SVDHVControlCallback(const NSMNode& node, const std::string& rcname)
  : HVControlCallback(node), m_rcnode(rcname), m_stopped_by_trip(false)
{
  getNode().setState(HVState::OFF_S);
}

void SVDHVControlCallback::init(NSMCommunicator&) throw()
{
  int status = ca_create_channel("B2:PSC:SVD:State:req:S", NULL, NULL, 0, &m_PSRqs);
  SEVCHK(status, "Create channel failed");
  status = ca_pend_io(1.0);
  SEVCHK(status, "Channel connection failed");
  status = ca_create_channel("B2:RC:SVD:State:req:S", NULL, NULL, 0, &m_RCRqs);
  SEVCHK(status, "Create channel failed");
  status = ca_pend_io(1.0);
  SEVCHK(status, "Channel connection failed");
  add(new NSMVHandlerText("hvstate", true, false, getNode().getState().getLabel()));
  addPV("B2:PSC:SVD:State:cur:S");
  addPV("B2:PSC:SVD:State:req:S");
  addPV("B2:RC:SVD:State:cur:S");
  addPV("B2:RC:SVD:State:req:S");
}

int SVDHVControlCallback::putPV(chid cid, const char* val)
{
  int status = 0;
  SEVCHK(ca_put(DBR_STRING, cid, val), "Put failed");
  ca_flush_io();
  return status;
}

void SVDHVControlCallback::turnon() throw(HVHandlerException)
{
  LogFile::info("turnon");
  putPV(m_PSRqs, "standby");
  putPV(m_RCRqs, "ready");
}

void SVDHVControlCallback::turnoff() throw(HVHandlerException)
{
  putPV(m_PSRqs, "off");
  putPV(m_RCRqs, "notReady");
}

void SVDHVControlCallback::standby() throw(HVHandlerException)
{
  putPV(m_RCRqs, "ready");
  putPV(m_PSRqs, "standby");
}

void SVDHVControlCallback::peak() throw(HVHandlerException)
{
  LogFile::info("peak");
  putPV(m_PSRqs, "peak");
}

void SVDHVControlCallback::recover() throw(HVHandlerException)
{
  putPV(m_PSRqs, "standby");
  putPV(m_RCRqs, "notReady");
}

bool SVDHVControlCallback::addPV(const std::string& pvname) throw()
{
  std::string vname = StringUtil::replace(pvname, ":", ".");
  try {
    getHandler(vname);
    return false;
  } catch (const std::out_of_range& e) {
    LogFile::debug("Adding new listening PV : %s", pvname.c_str());
  }
  char* pname = epicsStrDup(pvname.c_str());
  MYNODE* pvnode = new MYNODE;
  ca_create_channel(pname, connectionCallback,
                    pvnode, 20, &pvnode->mychid);
  ca_replace_access_rights_event(pvnode->mychid, accessRightsCallback);
  ca_create_subscription(DBR_STRING, 1, pvnode->mychid,
                         DBE_VALUE, eventCallback, pvnode, &pvnode->myevid);
  add(new SVDPSVHandler(vname, NSMVar("")));
  return true;
}

void SVDHVControlCallback::sendToRC(const RCCommand& cmd)
{
  NSMCommunicator::send(NSMMessage(m_rcnode, cmd));
}

SVDHVControlCallback::SVDPSVHandler::SVDPSVHandler(const std::string& name, const NSMVar& var)
  : NSMVHandler("", name, true, true)
{
  set(var);
  m_cid = NULL;
}

bool SVDHVControlCallback::SVDPSVHandler::handleGet(NSMVar& var)
{
  var = m_var;
  return true;
}

bool SVDHVControlCallback::SVDPSVHandler::handleSet(const NSMVar& var)
{
  m_var = var;
  std::string pvname = StringUtil::replace(var.getName(), ".", ":");
  chid cid;
  int status;
  if (m_cid == NULL) {
    status = ca_search(pvname.c_str(), &cid);
    if (status != ECA_NORMAL) return false;
    status = ca_pend_io(0.0);
    if (status != ECA_NORMAL) return false;
  } else {
    cid = m_cid;
  }
  switch (var.getType()) {
    case NSMVar::INT: {
      double v = var.getInt();
      status = ca_put(DBR_DOUBLE, cid, &v);
      break;
    }
    case NSMVar::FLOAT: {
      double v = var.getFloat();
      status = ca_put(DBR_DOUBLE, cid, &v);
      break;
    }
    case NSMVar::TEXT: {
      status = ca_put(DBR_STRING, cid, var.getText());
      break;
    }
    default:
      return false;
  }
  status = ca_pend_io(0.0);
  return (status == ECA_NORMAL);
}

