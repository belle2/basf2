//+
// File : RFMaster.cc
// Description : Master node of RFARM
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 31 - Jul - 2013
//-

#include "daq/rfarm/manager/RFMaster.h"

using namespace std;
using namespace Belle2;

RFMaster::RFMaster(string conffile)
{
  // 0. Initialize configuration manager
  m_conf = new RFConf(conffile.c_str());
  char* nodename = m_conf->getconf("master", "nodename");
  //  char nodename[256];
  //  gethostname ( nodename, sizeof(nodename) );

  // 2. Initialize local shared memory
  m_shm = new RFSharedMem(nodename);

  // 4. Set execution directory
  string execdir = string(m_conf->getconf("system", "execdir_base")) + "/master";

  mkdir(execdir.c_str(), 0755);
  chdir(execdir.c_str());

  // 5. Initialize LogManager
  m_log = new RFLogManager(nodename);

}

RFMaster::~RFMaster()
{
  delete m_log;
  delete m_shm;
  delete m_conf;
}


// Functions hooked up by NSM2

void RFMaster::Configure(NSMmsg*, NSMcontext*)
{
  int* pars;
  // 1. Configure distributor
  char* distributor = m_conf->getconf("distributor", "nodename");
  b2nsm_sendreq(distributor, "RF_CONFIGURE", 0, pars);
  sleep(2);

  // 2. Configure event processors
  int maxnodes = m_conf->getconfi("processor", "nnodes");
  int idbase = m_conf->getconfi("processor", "idbase");
  char* hostbase = m_conf->getconf("processor", "hostbase");
  char* badlist = m_conf->getconf("processor", "badlist");

  char hostnode[512], idname[3];
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    if (badlist == NULL  ||
        strstr(badlist, idname) == 0) {
      sprintf(hostnode, "evp_%s%2.2d", hostbase, idbase + i);
      b2nsm_sendreq(hostnode, "RF_CONFIGURE", 0, pars);
    }
  }
  sleep(2);

  // 3. Configure collector
  char* collector = m_conf->getconf("collector", "nodename");
  b2nsm_sendreq(collector, "RF_CONFIGURE", 0, pars);
}

void RFMaster::Start(NSMmsg*, NSMcontext*)
{
}

void RFMaster::Stop(NSMmsg*, NSMcontext*)
{
}


void RFMaster::Restart(NSMmsg*, NSMcontext*)
{
  int* pars;
  // 1. Configure distributor
  char* distributor = m_conf->getconf("distributor", "nodename");
  b2nsm_sendreq(distributor, "RF_RESTART", 0, pars);
  sleep(2);

  // 2. Configure event processors
  int maxnodes = m_conf->getconfi("processor", "nnodes");
  int idbase = m_conf->getconfi("processor", "idbase");
  char* hostbase = m_conf->getconf("processor", "hostbase");
  char* badlist = m_conf->getconf("processor", "badlist");

  char hostnode[512], idname[3];
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    if (badlist == NULL  ||
        strstr(badlist, idname) == 0) {
      sprintf(hostnode, "evp_%s%2.2d", hostbase, idbase + i);
      b2nsm_sendreq(hostnode, "RF_RESTART", 0, pars);
    }
  }
  sleep(2);

  // 3. Configure collector
  char* collector = m_conf->getconf("collector", "nodename");
  b2nsm_sendreq(collector, "RF_RESTART", 0, pars);

}

// Server function

void RFMaster::monitor_loop()
{
  while (true) {
    sleep(10);
  }
}



