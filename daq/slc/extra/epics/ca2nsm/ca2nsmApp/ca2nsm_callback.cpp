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

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Daemon.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

struct MYNODE {
  char value[20];
  chid mychid;
  evid myevid;
};

extern "C" {

  //static void printChidInfo(chid chid, const char *message);
  static void exceptionCallback(struct exception_handler_args args);
  static void connectionCallback(struct connection_handler_args args);
  static void accessRightsCallback(struct access_rights_handler_args args);
  static void eventCallback(struct event_handler_args eha);

}

namespace Belle2 {

  class CA2NSMCallback : public NSMCallback {

  private:
    struct PVInfo {
      std::string pvname;
      std::string pv;
      char* pname;
      MYNODE* pvnode;
    };

  private:
    StringList m_pvnames;

  public:
    CA2NSMCallback(const NSMNode& node, const StringList pvnames)
      : NSMCallback(), m_pvnames(pvnames)
    {
      setNode(node);
    }
    virtual ~CA2NSMCallback() throw() {}

  public:
    virtual void init(NSMCommunicator&) throw()
    {
      for (size_t i = 0; i < m_pvnames.size(); i++) {
	addPV(m_pvnames[i]);
      }
    }

  public:
    bool addPV(const std::string& pvname) throw()
    {
      try {
	getHandler(pvname);
	return false;
      } catch (const std::out_of_range& e) {
	LogFile::debug("Adding new listening PV : %s", pvname.c_str());
      }
      char* pname = epicsStrDup(pvname.c_str());
      MYNODE* pvnode = new MYNODE;
      SEVCHK(ca_create_channel(pname, connectionCallback,
			       pvnode, 20, &pvnode->mychid),
	     "ca_create_channel");
      SEVCHK(ca_replace_access_rights_event(pvnode->mychid, accessRightsCallback),
	     "ca_replace_access_rights_event");
      SEVCHK(ca_create_subscription(DBR_STRING,1, pvnode->mychid,
				    DBE_VALUE, eventCallback, pvnode, &pvnode->myevid),
	     "ca_create_subscription");
      add(new NSMVHandlerText(pvname, true, false, ""));
      return true;
    }

  };

}

static Belle2::CA2NSMCallback* g_callback = NULL;

using namespace Belle2;

void init_ca2nsm(const char* confname)
{
  ConfigFile config("slowcontrol", confname);
  const std::string hostname = config.get("nsm.host");
  const int port = config.getInt("nsm.port");
  const std::string nodename = config.get("nsm.nodename");
  int npvs = config.getInt("ca2nsm.npvs");
  StringList pvnames;
  for (int i = 0; i < npvs; i++) {
    std::string pvname = config.get(StringUtil::form("ca2nsm.pv[%d]", i));
    if (pvname.size() > 0) {
      pvname = StringUtil::tolower(StringUtil::replace(pvname, ":", "."));
      pvnames.push_back(pvname);
    }
  }
  g_callback = new CA2NSMCallback(NSMNode(nodename), pvnames);
  PThread(new NSMNodeDaemon(g_callback, hostname, port));
}

void add_ca2nsm(const char* pvname)
{
  g_callback->addPV(pvname);
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

/*
void exceptionCallback(struct exception_handler_args args)
{
  chid chid = args.chid;
  long stat = args.stat; 
  const char* channel;
  channel = (chid ? ca_name(chid) : g_noname);
  if(chid) printChidInfo(chid, "exceptionCallback");
  LogFile::debug("exceptionCallback stat %s channel %s",
		 ca_message(stat),channel);
}
*/

void connectionCallback(struct connection_handler_args args)
{
  chid chid = args.chid;
  printChidInfo(chid,"connectionCallback");
  LogFile::debug("%s:%d",__FILE__, __LINE__);
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
      std::string pvname = StringUtil::tolower(StringUtil::replace(pvname_c, ":", "."));
      LogFile::debug("Event Callback: %s = %s", pvname.c_str(), pvdata);
      g_callback->set(pvname, pvdata);
    } else {
      LogFile::warning("Unknown PV (chid=%d)", eha.chid);
    }
  }
}
