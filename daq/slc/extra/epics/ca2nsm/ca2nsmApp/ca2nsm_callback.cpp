#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"

extern "C" {
#include "ca2nsm.h"
#include "ca2nsm_callback.h"
}

#include "CA2NSMCallback.h"

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

static Belle2::CA2NSMCallback* g_callback = NULL;

using namespace Belle2;

void init_ca2nsm(const char* confname)
{
  ConfigFile config("slowcontrol", confname);
  const std::string hostname = config.get("nsm.host");
  const int port = config.getInt("nsm.port");
  const std::string nodename = config.get("nsm.nodename");
  StringList pvnames;
  for (int i = 0; true; i++) {
    std::string pvname = config.get(StringUtil::form("ca2nsm.pv[%d]", i));
    if (pvname.size() > 0) {
      pvnames.push_back(pvname);
    } else {
      break;
    }
  }
  SEVCHK(ca_context_create(ca_enable_preemptive_callback),"ca_context_create");
  g_callback = new CA2NSMCallback(NSMNode(nodename), pvnames);
  PThread(new NSMNodeDaemon(g_callback, hostname, port));
}

void add_ca2nsm(const char* pvname)
{
  g_callback->addPV(pvname);
}

void add_PVs()
{
  g_callback->addPVs();
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
      g_callback->set(pvname, pvdata);
      g_callback->notify();
    } else {
      LogFile::warning("Unknown PV (chid=%d)", eha.chid);
    }
  }
}
