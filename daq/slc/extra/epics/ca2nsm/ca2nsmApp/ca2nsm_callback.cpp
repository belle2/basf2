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

  static void printChidInfo(chid chid, const char *message);
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

  public:
    CA2NSMCallback(const NSMNode& node) 
      : NSMCallback(node) { 
    }
    virtual ~CA2NSMCallback() throw() {}

  public:
    void init() throw()
    {
      m_data = NSMData(getNode().getName() + "_STATUS", "ca2nsm", ca2nsm_revision);
      ca2nsm* nsm = (ca2nsm*)m_data.allocate(getCommunicator());
      memset(nsm, 0, sizeof(ca2nsm));
      SEVCHK(ca_context_create(ca_disable_preemptive_callback),
	     "ca_context_create");
      SEVCHK(ca_add_exception_event(exceptionCallback, NULL),
	     "ca_add_exception_event");
    }

  public:
    virtual NSMVar vget(const std::string& pvname) throw()
    {
      ca2nsm* nsm = (ca2nsm*)m_data.get();
      for (int i = 0; i < MAX_PV; i++) {
	ca2nsm::pv_info& info(nsm->pv[i]);
        if (pvname == info.name) {
 	  return NSMVar::NOVALUE;
        }
      }
      char* pname = epicsStrDup(pvname.c_str());
      MYNODE* pvnode = new MYNODE;
      SEVCHK(ca_create_channel(pname, connectionCallback,
			       pvnode, 20, &pvnode->mychid),
	     "ca_create_channel");
      SEVCHK(ca_replace_access_rights_event(pvnode->mychid,
					    accessRightsCallback),
	     "ca_replace_access_rights_event");
      SEVCHK(ca_create_subscription(DBR_STRING,1, pvnode->mychid,
				    DBE_VALUE, eventCallback, pvnode,
				    &pvnode->myevid),
	     "ca_create_subscription");
      for (int i = 0; i < MAX_PV; i++) {
	ca2nsm::pv_info& info(nsm->pv[i]);
	if (info.chid == 0) {
	  memset(&info, 0, sizeof(info));
	  info.chid = i + 1;
	  strcpy(info.name, pvname.c_str());
	  break;
	}
      }      
      return NSMVar::NOVALUE;
    }

    void returnPV(const std::string& pvname, const std::string& pv)
    {
      ca2nsm* nsm = (ca2nsm*)m_data.get();
      for (int i = 0; i < MAX_PV; i++) {
	ca2nsm::pv_info& info(nsm->pv[i]);
	if (pvname == info.name) {
	  strcpy(info.data, pv.c_str());
	  info.timestamp = Time().getSecond();
	}
      }
    }
    
  private:
    NSMData m_data;

  };

}

static Belle2::CA2NSMCallback* g_callback = NULL;
static const char* g_noname = "unknown";

void init_ca2nsm(const char* confname)
{
  using namespace Belle2;
  ConfigFile config("slowcontrol", confname);
  const std::string hostname = config.get("nsm.local.host");
  const int port = config.getInt("nsm.local.port");
  const std::string nodename = config.get("nsm.nodename");
  g_callback = new CA2NSMCallback(NSMNode(nodename));
  int npvs = config.getInt("ca2nsm.npvs");
  PThread(new NSMNodeDaemon(g_callback, hostname, port));
  for (int i = 0; i < npvs; i++) {
    std::string pvname = config.get(StringUtil::form("ca2nsm.pv[%d]", i));
    if (pvname.size() > 0) {
      g_callback->vget(pvname);
    }
  }
}

void add_ca2nsm(const char* pvname)
{
  using namespace Belle2;
  g_callback->vget(pvname);
}

void printChidInfo(chid chid, const char* message)
{
  /*
  printf("\n%s\n",message);
  printf("pv: %s  type(%d) nelements(%ld) host(%s)",
	 ca_name(chid),ca_field_type(chid),ca_element_count(chid),
	 ca_host_name(chid));
  printf(" read(%d) write(%d) state(%d)\n",
	 ca_read_access(chid),ca_write_access(chid),ca_state(chid));
  */
}

void exceptionCallback(struct exception_handler_args args)
{
  /*
  chid chid = args.chid;
  long stat = args.stat; 
  const char* channel;
  channel = (chid ? ca_name(chid) : g_noname);
  if(chid) printChidInfo(chid, "exceptionCallback");
  printf("exceptionCallback stat %s channel %s\n",
	 ca_message(stat),channel);
  */
  /* Channel access status code*/
}

void connectionCallback(struct connection_handler_args args)
{
  chid chid = args.chid;
  //printChidInfo(chid,"connectionCallback");
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
    const char* pvname = ca_name(eha.chid);
    printf("Event Callback: %s = %s\n", pvname, pvdata);
    g_callback->returnPV(pvname, pvdata);
  }
}
