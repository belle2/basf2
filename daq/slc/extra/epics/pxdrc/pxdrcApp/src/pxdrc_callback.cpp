#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"

extern "C" {
#include "pxdrc_callback.h"
}

#include "PXDRCCallback.h"

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

static Belle2::PXDRCCallback* g_callback = NULL;

using namespace Belle2;

void init_pxdrc(const char* confname)
{
  ConfigFile config("slowcontrol", confname);
  const std::string hostname = config.get("nsm.host");
  const int port = config.getInt("nsm.port");
  const std::string nodename = config.get("nsm.nodename");
  SEVCHK(ca_context_create(ca_enable_preemptive_callback), "ca_context_create");
  g_callback = new PXDRCCallback(NSMNode(nodename));
  PThread(new NSMNodeDaemon(g_callback, hostname, port));
}

void printChidInfo(chid chid, const char* message)
{
  LogFile::debug("%s", message);
  LogFile::debug("pv: %s  type(%d) nelements(%ld) host(%s)",
                 ca_name(chid), ca_field_type(chid), ca_element_count(chid),
                 ca_host_name(chid));
  LogFile::debug(" read(%d) write(%d) state(%d)",
                 ca_read_access(chid), ca_write_access(chid), ca_state(chid));
}

void connectionCallback(struct connection_handler_args args)
{
  chid chid = args.chid;
  printChidInfo(chid, "connectionCallback");
  // if the channel :req or :cur gets disconnected, translate that back to a disconnected on the nsm side!
  if (args.op == CA_OP_CONN_DOWN) {
    // will not harm to report UNKNOWN for either two disconnects, as they are one the same gw/ioc. if we cannot send request its as awful as if cannot get actual status.
    g_callback->setState(RCState::UNKNOWN);
  }
}

void accessRightsCallback(struct access_rights_handler_args args)
{
  chid chid = args.chid;
  printChidInfo(chid, "accessRightsCallback");
}

void eventCallback(struct event_handler_args eha)
{
  chid  chid = eha.chid;
  if (eha.status != ECA_NORMAL) {
    printChidInfo(chid, "eventCallback");
  } else {
    const char* pvdata  = (const char*)eha.dbr;
    const char* pvname_c = ca_name(eha.chid);
    if (pvname_c != NULL) {
      std::string pvname = StringUtil::replace(pvname_c, ":", ".");
      g_callback->set(pvname, pvdata);
      LogFile::debug("Event Callback: %s = %s", pvname.c_str(), pvdata);
      std::string val = pvdata;
      if (pvname == "B2.PXD.RC.State.cur.S") {
        if (val == "NOTREADY") {
          g_callback->setState(RCState::NOTREADY_S);
        } else if (val == "READY") {
          g_callback->setState(RCState::READY_S);
        } else if (val == "RUNNING") {
          g_callback->setState(RCState::RUNNING_S);
        } else if (val == "LOADING") {
          g_callback->setState(RCState::LOADING_TS);
        } else if (val == "UNLOADING") {
          g_callback->setState(RCState::ABORTING_RS); // There is no unloading state yet
        } else if (val == "STARTING") {
          g_callback->setState(RCState::STARTING_TS);
        } else if (val == "STOPPING") {
          g_callback->setState(RCState::STOPPING_TS);
        } else if (val == "UNKNOWN") {
          g_callback->setState(RCState::UNKNOWN);
        } else if (val == "ERROR") {
          g_callback->setState(RCState::ERROR_ES);
        }
      } else if (strcmp(pvname_c, PXDRCCallback::pvRCreq) == 0) {
        // That IS confusing, why should we (re)set nsm req state which was
        // used to set this PV? (kindof loop)
        // even if the PV is changed on the PXDRC (local operation), I do not see a reason to
        // report that back in this way.
        // if (val == "NOTREADY") {
        //   g_callback->setStateRequest(RCState::NOTREADY_S);
        // } else if (val == "READY") {
        //   g_callback->setStateRequest(RCState::READY_S);
        // } else if (val == "RUNNING") {
        //   g_callback->setStateRequest(RCState::RUNNING_S);
        // }
      }
    } else {
      LogFile::warning("Unknown PV (chid=%d)", eha.chid);
    }
  }
}
