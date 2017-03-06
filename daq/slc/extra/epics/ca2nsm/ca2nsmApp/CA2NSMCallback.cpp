#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"

#include "CA2NSMCallback.h"

#include <daq/slc/nsm/NSMCallback.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void printChidInfo(chid chid, const char* message);
void connectionCallback(struct connection_handler_args args);
void accessRightsCallback(struct access_rights_handler_args args);
void eventCallback(struct event_handler_args eha);

CA2NSMVHandler::CA2NSMVHandler(const std::string& name, const NSMVar& var)
  : NSMVHandler("", name, true, true)
{
  set(var);
  m_cid = NULL;
}

bool CA2NSMVHandler::handleGet(NSMVar& var)
{
  var = m_var;
  return true;
}

bool CA2NSMVHandler::handleSet(const NSMVar& var)
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

CA2NSMCallback::CA2NSMCallback(const NSMNode& node, const StringList pvnames)
  : NSMCallback(), m_pvnames(pvnames)
{
  setNode(node);
}

void CA2NSMCallback::init(NSMCommunicator&) throw()
{
  for (size_t i = 0; i < m_pvnames.size(); i++) {
    addPV(m_pvnames[i]);
  }
}

void CA2NSMCallback::vset(NSMCommunicator& com, const NSMVar& var) throw()
{
  try {
    getHandler(var.getName());
  } catch (const std::out_of_range& e) {
    add(new CA2NSMVHandler(var.getName(), var));
  }
  NSMCallback::vset(com, var);
}

void CA2NSMCallback::vget(const std::string& nodename,
                          const std::string& vname) throw()
{
  std::string pvname = StringUtil::replace(vname, ".", ":");
  try {
    getHandler(vname);
  } catch (const std::out_of_range& e) {
    m_pvmutex.lock();
    m_pvnames_reserved.push_back(pvname);
    m_pvcond.wait(m_pvmutex, 1);
    m_pvmutex.unlock();
  }
  NSMCallback::vget(nodename, vname);
}

void CA2NSMCallback::addPVs() throw()
{
  for (size_t i = 0; i < m_pvnames_reserved.size(); i++) {
    addPV(m_pvnames_reserved[i]);
  }
  m_pvmutex.lock();
  m_pvnames_reserved = StringList();
  m_pvmutex.unlock();
}

bool CA2NSMCallback::addPV(const std::string& pvname) throw()
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
  add(new CA2NSMVHandler(vname, NSMVar("")));
  return true;
}

