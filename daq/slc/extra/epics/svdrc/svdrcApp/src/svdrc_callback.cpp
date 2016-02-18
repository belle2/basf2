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

#include "SVDRCCallback.h"

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMNodeDaemon.h>

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

using namespace Belle2;

void init_svdrc(const char* confname)
{
  ConfigFile config("slowcontrol", confname);
  const std::string hostname = config.get("nsm.host");
  const int port = config.getInt("nsm.port");
  const std::string nodename = config.get("nsm.nodename");
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
    const char* pvdata  = (const char *)eha.dbr;
    const char* pvname_c = ca_name(eha.chid);
    if (pvname_c != NULL) {
      std::string pvname = StringUtil::replace(pvname_c, ":", ".");
      LogFile::debug("Event Callback: %s = %s", pvname.c_str(), pvdata);
      RCState state_target = g_callback->getStateTarget();
      g_callback->set(pvname, pvdata);
      std::string val = pvdata;
      if (pvname == "GPSC.SVD.State.Act") {
	if (val == "NotReady") {
	  g_callback->setState(RCState::NOTREADY_S);
	} else if (val == "Ready") {
	  g_callback->setState(RCState::READY_S);
	} else if (val == "Running") {
	  g_callback->setState(RCState::RUNNING_S);
	} else if (val == "Loading") {
	  g_callback->setState(RCState::LOADING_TS);
	} else if (val == "Unloading") {
	  g_callback->setState(RCState::ABORTING_RS);
	} else if (val == "Unknown") {
	  g_callback->setState(RCState::UNKNOWN);
	} else if (val == "Error") {
	  g_callback->setState(RCState::ERROR_ES);
	}
      } else if (pvname == "GPSC.SVD.State.Rqs") {
      } 
    } else {
      LogFile::warning("Unknown PV (chid=%d)", eha.chid);
    }
  }
}
