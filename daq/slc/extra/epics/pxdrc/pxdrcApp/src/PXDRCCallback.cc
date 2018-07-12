#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"

#include "PXDRCCallback.h"

#include <daq/slc/nsm/NSMCallback.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

const char* PXDRCCallback::pvRCcur = "PXD:RC:State:cur:S";
const char* PXDRCCallback::pvRCreq = "PXD:RC:State:req:S";

void printChidInfo(chid chid, const char* message);
void connectionCallback(struct connection_handler_args args);
void accessRightsCallback(struct access_rights_handler_args args);
void eventCallback(struct event_handler_args eha);

#include <cadef.h>

PXDRCCallback::PXDRCCallback(const NSMNode& node)
  : RCCallback()
{
  m_RC_req = NULL; // make sure we dont use it before they are defined!
  m_RC_cur = NULL; // make sure we dont use it before they are defined!
  setNode(node);
  setAutoReply(false);
}

void PXDRCCallback::init(NSMCommunicator&) throw()
{
  NSMNode& node(getNode());
  node.setState(RCState::NOTREADY_S);
  add(new NSMVHandlerText("rcstate", true, false, node.getState().getLabel()));
  add(new NSMVHandlerText("rcconfig", true, false, "default"));
  add(new NSMVHandlerText("dbtable", true, false, "none"));
  addPV(pvRCreq, m_RC_req);
  addPV(pvRCcur, m_RC_cur);
}

int PXDRCCallback::putPV(chid cid, const char* val)
{
  int status = 0;
  LogFile::info("put PV >> %s", val);
  SEVCHK(ca_put(DBR_STRING, cid, val), "Put failed");
  ca_flush_io();
  return status;
}

RCState PXDRCCallback::getRCCurrent()
{
  char pvalue[100];
  ca_get(DBR_STRING, m_RC_cur, pvalue);
  return RCState(pvalue);
}

RCState PXDRCCallback::getRCRequest()
{
  char pvalue[100];
  ca_get(DBR_STRING, m_RC_req, pvalue);
  return RCState(pvalue);
}

void PXDRCCallback::load(const DBObject&) throw(RCHandlerException)
{
  if (m_state_req == RCState::NOTREADY_S) {
    putPV(m_RC_req, "READY");
  } else if (m_state_req == RCState::READY_S) {
    setState(RCState::READY_S);
  }
}

void PXDRCCallback::abort() throw(RCHandlerException)
{
  if (m_state_req == RCState::NOTREADY_S) {
    setState(RCState::NOTREADY_S);
  } else {
    putPV(m_RC_req, "NOTREADY");
  }
}

void PXDRCCallback::start(int expno, int runno) throw(RCHandlerException)
{
  putPV(m_RC_req, "RUNNING");
}

void PXDRCCallback::stop() throw(RCHandlerException)
{
  putPV(m_RC_req, "READY");
}

bool PXDRCCallback::addPV(const std::string& pvname, chid& mychid) throw()
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
  int status = ca_create_channel(pname, connectionCallback,
                                 pvnode, 20, &pvnode->mychid);
  SEVCHK(status, "Create channel failed");
  mychid = pvnode->mychid;
  status = ca_replace_access_rights_event(pvnode->mychid, accessRightsCallback);
  SEVCHK(status, "Replace Channel access failed");
  status = ca_create_subscription(DBR_STRING, 1, pvnode->mychid,
                                  DBE_VALUE, eventCallback, pvnode, &pvnode->myevid);
  SEVCHK(status, "Create channel subscription failed");
  add(new PXDRCVHandler(vname, NSMVar("")));
  return true;
}

PXDRCCallback::PXDRCVHandler::PXDRCVHandler(const std::string& name, const NSMVar& var)
  : NSMVHandler("", name, true, true)
{
  set(var);
  m_cid = NULL;
}

bool PXDRCCallback::PXDRCVHandler::handleGet(NSMVar& var)
{
  var = m_var;
  return true;
}

bool PXDRCCallback::PXDRCVHandler::handleSet(const NSMVar& var)
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

