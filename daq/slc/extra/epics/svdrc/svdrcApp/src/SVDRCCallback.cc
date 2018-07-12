#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"

#include "SVDStateDefs.h"
#include "SVDRCCallback.h"

#include <daq/slc/nsm/NSMCallback.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void printChidInfo(chid chid, const char* message);
void connectionCallback(struct connection_handler_args args);
void accessRightsCallback(struct access_rights_handler_args args);
void eventCallback(struct event_handler_args eha);

SVDRCCallback::SVDRCCallback(const NSMNode& node)
  : RCCallback()
{
  setNode(node);
  setAutoReply(false);
}

void SVDRCCallback::init(NSMCommunicator&) throw()
{
  int status = ca_create_channel("SVD:CTRL:Request", NULL, NULL, 0, &m_RC_req);
  //status = ca_create_channel("B2:RC:SVD:State:req:S", NULL, NULL, 0, &m_RCRqs);
  SEVCHK(status, "Create channel failed");
  status = ca_pend_io(1.0);
  SEVCHK(status, "Channel connection failed");
  NSMNode& node(getNode());
  node.setState(RCState::NOTREADY_S);
  add(new NSMVHandlerText("rcstate", true, false, node.getState().getLabel()));
  add(new NSMVHandlerText("rcconfig", true, false, "default"));
  add(new NSMVHandlerText("dbtable", true, false, "none"));
  addPV("SVD:CTRL:Request");
  addPV("SVD:CTRL:State");
  addPV("SVD:FADC:RunMode");
}

int SVDRCCallback::putPV(chid cid, const char* val)
{
  int status = 0;
  SEVCHK(ca_put(DBR_STRING, cid, val), "Put failed");
  ca_flush_io();
  return status;
}

int SVDRCCallback::putPV(chid cid, int val)
{
  int status = 0;
  double dval = val;
  SEVCHK(ca_put(DBR_DOUBLE, cid, &dval), "Put failed");
  ca_flush_io();
  return status;
}

void SVDRCCallback::load(const DBObject&) throw(RCHandlerException)
{
  if (m_state_req == RCState::READY_S) {
    setState(RCState::READY_S);
  } else {
    putPV(m_RC_req, MAIN_REQ_CONFIGURE);
  }
}

void SVDRCCallback::abort() throw(RCHandlerException)
{
  if (m_state_req == RCState::NOTREADY_S) {
    setState(RCState::NOTREADY_S);
  } else {
    putPV(m_RC_req, MAIN_REQ_ABORT);
  }
}

void SVDRCCallback::start(int expno, int runno) throw(RCHandlerException)
{
  //putPV(m_RCRqs, "running");
  putPV(m_RC_req, MAIN_REQ_START);
}

void SVDRCCallback::stop() throw(RCHandlerException)
{
  //putPV(m_RCRqs, "ready");
  putPV(m_RC_req, MAIN_REQ_STOP);
}

bool SVDRCCallback::addPV(const std::string& pvname) throw()
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
  add(new SVDRCVHandler(vname, NSMVar("", "")));
  return true;
}

SVDRCCallback::SVDRCVHandler::SVDRCVHandler(const std::string& name, const NSMVar& var)
  : NSMVHandler("", name, true, true)
{
  set(var);
  m_cid = NULL;
}

bool SVDRCCallback::SVDRCVHandler::handleGet(NSMVar& var)
{
  var = m_var;
  return true;
}

bool SVDRCCallback::SVDRCVHandler::handleSet(const NSMVar& var)
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

