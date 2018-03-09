#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"

extern "C" {
#include "svdrc_callback.h"
}

#include "SVDStateDefs.h"
#include "SVDRCCallback.h"

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMNode.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Daemon.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/Cond.h>
#include <daq/slc/system/Mutex.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <map>

static Belle2::SVDRCCallback* g_callback = NULL;
Belle2::NSMNode g_rcnode;

using namespace Belle2;

void init_svdrc(const char* confname)
{
  ConfigFile config("slowcontrol", confname);
  const std::string hostname = config.get("nsm.host");
  const int port = config.getInt("nsm.port");
  const std::string nodename = config.get("nsm.nodename");
  g_rcnode = NSMNode(config.get("rcnode"));
  SEVCHK(ca_context_create(ca_enable_preemptive_callback),"ca_context_create");
  g_callback = new SVDRCCallback(NSMNode(nodename));
  PThread(new NSMNodeDaemon(g_callback, hostname, port));
}

void printChidInfo(chid chid, const char* message)
{
  LogFile::debug("%s",message);
  LogFile::debug("pv: %s  type(%d) nelements(%ld) host(%s)",
		 ca_name(chid),ca_field_type(chid),ca_element_count(chid),
		 ca_host_name(chid));
  LogFile::debug(" read(%d) write(%d) state(%d)",
		 ca_read_access(chid),ca_write_access(chid),ca_state(chid));
}

void connectionCallback(struct connection_handler_args args)
{
  chid chid = args.chid;
  printChidInfo(chid,"connectionCallback");
}

void accessRightsCallback(struct access_rights_handler_args args)
{
  chid chid = args.chid;
  printChidInfo(chid,"accessRightsCallback");
}

void eventCallback(struct event_handler_args eha)
{
  chid	chid = eha.chid;
  if(eha.status != ECA_NORMAL) {
    printChidInfo(chid, "eventCallback");
  } else {
    const std::string pvdata  = StringUtil::toupper((const char *)eha.dbr);
    const char* pvname_c = ca_name(eha.chid);
    if (pvname_c != NULL) {
      std::string pvname = StringUtil::replace(pvname_c, ":", ".");
      //int ival = atoi(pvdata.c_str());
      LogFile::info("Event Callback: %s = %s", pvname.c_str(), pvdata.c_str());
      if (pvname == "SVD.CTRL.State") {
	if (pvdata == "IDLE") {
	  g_callback->setState(RCState::NOTREADY_S);
	  g_callback->set(pvname, "IDLE");
	} else if (pvdata == "DOWN") {
	  g_callback->setState(RCState::NOTREADY_S);
	  g_callback->set(pvname, "DOWN");
	} else if (pvdata == "CONFIGURING") {
	  g_callback->setState(RCState::LOADING_TS);
	  g_callback->set(pvname, "CONFIGURING");
	} else if (pvdata == "READY") {
	  g_callback->setState(RCState::READY_S);
	  g_callback->set(pvname, "READY");
	} else if (pvdata == "EXITED") {
	  g_callback->setState(RCState::UNKNOWN);
	  g_callback->set(pvname, "EXITED");
	} else if (pvdata == "FINISHED") {
	  g_callback->setState(RCState::NOTREADY_S);
	  g_callback->set(pvname, "FINISHED");
	} else if (pvdata == "FINISHING") {
	  g_callback->setState(RCState::ABORTING_RS);
	  g_callback->set(pvname, "FINISHING");
	} else if (pvdata == "RUNNING") {
	  g_callback->setState(RCState::RUNNING_S);
	  g_callback->set(pvname, "RUNNING");
	} else if (pvdata == "ABORTING") {
	  g_callback->setState(RCState::ABORTING_RS);
	  g_callback->set(pvname, "ABORTING");
	} else if (pvdata == "ERROR") {
	  g_callback->setState(RCState::ERROR_ES);
	  g_callback->set(pvname, "ERROR");
	}
      } else if (pvname == "SVD.CTRL.Request") {
	if (pvdata == "PROCESSED") {
	  g_callback->set(pvname, "PROCESSED");
	} else if (pvdata == "CONFIGURE") {
	  g_callback->setStateRequest(RCState::READY_S);
	  g_callback->set(pvname, "CONFIGURE");
	} else if (pvdata == "START") {
	  g_callback->setStateRequest(RCState::RUNNING_S);
	  g_callback->set(pvname, "START");
	} else if (pvdata == "STOP") {
	  g_callback->setStateRequest(RCState::READY_S);
	  g_callback->set(pvname, "STOP");
	} else if (pvdata == "ABORT") {	
	  g_callback->setStateRequest(RCState::NOTREADY_S);
	  g_callback->set(pvname, "ABORT");
	}
      } else if (pvname == "SVD.FADC.RunMode") {
	std::string vname = StringUtil::tolower(g_callback->getNode().getName())+".used";
	LogFile::info("%s.%s>>%s", g_rcnode.getName().c_str(), vname.c_str(), (pvdata == "GLOBAL RUN"?"on":"off"));
	try {
	  //  g_callback->set(g_rcnode, vname, (int)(pvdata == "GLOBAL RUN"));
	} catch (const TimeoutException& e) {
	  
	}
	g_callback->set(pvname, pvdata);
      } else {//if (pvname == "B2.RC.SVD.State.cur.S"") {
	g_callback->set(pvname, pvdata);
      } 
    } else {
      LogFile::warning("Unknown PV (chid=%d)", eha.chid);
    }
  }
}
