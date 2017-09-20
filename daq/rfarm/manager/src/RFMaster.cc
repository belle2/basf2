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

//#define DESY

// Main
RFMaster::RFMaster(string conffile)
{
  // 0. Initialize configuration manager
  m_conf = new RFConf(conffile.c_str());
  char* nodename = m_conf->getconf("master", "nodename");
  //  char nodename[256];
  //  gethostname ( nodename, sizeof(nodename) );

  // 1. Initialize local shared memory
  //  m_shm = new RFSharedMem(nodename);

  // 2. Set execution directory
  string execdir = string(m_conf->getconf("system", "execdir_base")) + "/master";

  mkdir(execdir.c_str(), 0755);
  chdir(execdir.c_str());

  // 3. Initialize LogManager
  m_log = new RFLogManager(nodename, m_conf->getconf("system", "lognode"));

  // 4. Leave PID file
  FILE* f = fopen("pid.data", "w");
  fprintf(f, "%d", getpid());
  fclose(f);

}

RFMaster::~RFMaster()
{
  delete m_log;
  //  delete m_shm;
  delete m_conf;
}

void RFMaster::Hook_Message_Handlers()
{
  // 5. Hook message handlers
  if (b2nsm_callback("LOG", Log_Handler) < 0) {
    fprintf(stderr, "RFMaster : hooking INFO handler failed, %s\n",
            b2nsm_strerror());
  }
  printf("RFMaster: Message Handlers - Ready\n");

}

// NSM callback functions for message

void RFMaster::Log_Handler(NSMmsg* msg, NSMcontext* ctx)
{
  //  printf ( "RFMaster : [INFO] received\n" );
  //  b2nsm_ok ( msg, "INFO!!", NULL );
  //  fflush ( stdout );
}


// Functions hooked up by NSM2

int RFMaster::Configure(NSMmsg*, NSMcontext*)
{
  int* pars;

  // 0. Configure DqmServer
  char* dqmserver = m_conf->getconf("dqmserver", "nodename");
  RFNSM_Status::Instance().set_flag(0);
  //  b2nsm_sendreq(dqmserver, "RF_CONFIGURE", 0, pars);
  b2nsm_sendreq(dqmserver, "RC_LOAD", 0, pars);
  while (RFNSM_Status::Instance().get_flag() == 0) b2nsm_wait(1);
  printf("RFMaster:: dqmserver configured\n");
  //  sleep(2);

  // 1. Configure distributor
  char* distributor = m_conf->getconf("distributor", "nodename");
  RFNSM_Status::Instance().set_flag(0);
  //  b2nsm_sendreq(distributor, "RF_CONFIGURE", 0, pars);
  b2nsm_sendreq(distributor, "RC_LOAD", 0, pars);
  while (RFNSM_Status::Instance().get_flag() == 0) b2nsm_wait(1);
  printf("RFMaster:: distributor configured\n");

  //  sleep(2);

  // 2. Configure event processors
  int maxnodes = m_conf->getconfi("processor", "nnodes");
  int idbase = m_conf->getconfi("processor", "idbase");
  char* hostbase = m_conf->getconf("processor", "nodebase");
  char* badlist = m_conf->getconf("processor", "badlist");

  char hostnode[512], idname[3];
  int nnodes = 0;
  RFNSM_Status::Instance().set_flag(0);
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    if (badlist == NULL  ||
        strstr(badlist, idname) == 0) {
      sprintf(hostnode, "evp_%s%2.2d", hostbase, idbase + i);
      //      b2nsm_sendreq(hostnode, "RF_CONFIGURE", 0, pars);
      b2nsm_sendreq(hostnode, "RC_LOAD", 0, pars);
      nnodes++;
    }
  }
  //  while (RFNSM_Status::Instance().get_flag() != nnodes) b2nsm_wait(1);
  sleep(10);

  printf("RFMaster:: distributor configured\n");

  // 3. Configure collector
  char* collector = m_conf->getconf("collector", "nodename");
  RFNSM_Status::Instance().set_flag(0);
  //  b2nsm_sendreq(collector, "RF_CONFIGURE", 0, pars);
  b2nsm_sendreq(collector, "RC_LOAD", 0, pars);
  while (RFNSM_Status::Instance().get_flag() == 0) b2nsm_wait(1);
  printf("RFMaster:: collector configured\n");
  //  sleep ( 5 );

  // 4. Configure
  char* roisender = m_conf->getconf("roisender", "nodename");
  RFNSM_Status::Instance().set_flag(0);
  //  b2nsm_sendreq(roisender, "RF_CONFIGURE", 0, pars);
  b2nsm_sendreq(roisender, "RC_LOAD", 0, pars);
  while (RFNSM_Status::Instance().get_flag() == 0) b2nsm_wait(1);
  printf("RFMaster:: roisender configured\n");

  return 0;
}

int RFMaster::UnConfigure(NSMmsg* msgm, NSMcontext* msgc)
{
  int pars[10];
  pars[0] = msgm->pars[0];
  pars[1] = msgm->pars[1];

  // Unconfigure RoiSender
  char* roisender = m_conf->getconf("roisender", "nodename");
  RFNSM_Status::Instance().set_flag(0);
  //  b2nsm_sendreq(roisender, "RF_UNCONFIGURE", 0, pars);
  b2nsm_sendreq(roisender, "RC_ABORT", 0, pars);
  while (RFNSM_Status::Instance().get_flag() == 1) b2nsm_wait(1);

  // Unconfigure collector
  char* collector = m_conf->getconf("collector", "nodename");
  RFNSM_Status::Instance().set_flag(0);
  //  b2nsm_sendreq(collector, "RF_UNCONFIGURE", 0, pars);
  b2nsm_sendreq(collector, "RC_ABORT", 0, pars);
  while (RFNSM_Status::Instance().get_flag() == 1) b2nsm_wait(1);

  // Unconfigure event processors
  int maxnodes = m_conf->getconfi("processor", "nnodes");
  int idbase = m_conf->getconfi("processor", "idbase");
  char* hostbase = m_conf->getconf("processor", "nodebase");
  char* badlist = m_conf->getconf("processor", "badlist");

  char hostnode[512], idname[3];
  RFNSM_Status::Instance().set_flag(0);
  int nnodes = 0;
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    if (badlist == NULL  ||
        strstr(badlist, idname) == 0) {
      sprintf(hostnode, "evp_%s%2.2d", hostbase, idbase + i);
      //      b2nsm_sendreq(hostnode, "RF_UNCONFIGURE", 0, pars);
      b2nsm_sendreq(hostnode, "RC_ABORT", 0, pars);
      nnodes++;
    }
  }
#ifdef DESY
  b2nsm_wait(5);
#else
  //  printf ( "RFMaster : Unconfigure : started. - nnodes=%d, flag=%d\n",
  //     nnodes, RFNSM_Status::Instance().get_flag() );
  while (RFNSM_Status::Instance().get_flag() != nnodes) {
    b2nsm_wait(1);
    //    printf ( "RFMaster : Unconfigure : in prog. - nnodes=%d, flag=%d\n",
    //       nnodes, RFNSM_Status::Instance().get_flag() );
  }
  //  printf ( "RFMaster : Unconfigure - done: nnodes=%d, flag=%d\n",
  //       nnodes, RFNSM_Status::Instance().get_flag() );
#endif

  // Unconfigure DqmServer
  char* dqmserver = m_conf->getconf("dqmserver", "nodename");
  RFNSM_Status::Instance().set_flag(0);
  //  b2nsm_sendreq(dqmserver, "RF_UNCONFIGURE", 0, pars);
  b2nsm_sendreq(dqmserver, "RC_ABORT", 0, pars);
  while (RFNSM_Status::Instance().get_flag() == 0) b2nsm_wait(1);

  // Unconfigure distributor
  char* distributor = m_conf->getconf("distributor", "nodename");
  RFNSM_Status::Instance().set_flag(0);
  //  b2nsm_sendreq(distributor, "RF_UNCONFIGURE", 0, pars);
  b2nsm_sendreq(distributor, "RC_ABORT", 0, pars);
  while (RFNSM_Status::Instance().get_flag() == 0) b2nsm_wait(1);

  printf("RFMaster : Unconfigure done.\n");
  fflush(stdout);

  return 0;

}

int RFMaster::Start(NSMmsg* msgm, NSMcontext* msgc)
{
  int pars[10];
  pars[0] = msgm->pars[0];
  pars[1] = msgm->pars[1];
  printf("RFMaster : Start exp=%d, run=%d\n", pars[0], pars[1]);
  fflush(stdout);

  // 0. Start DQMserver
  char* dqmserver = m_conf->getconf("dqmserver", "nodename");
  RFNSM_Status::Instance().set_flag(0);
  //  b2nsm_sendreq(dqmserver, "RF_STOP", 0, pars);
  b2nsm_sendreq(dqmserver, "RC_START", 2, pars);
  while (RFNSM_Status::Instance().get_flag() == 0) b2nsm_wait(1);

  // 1. Start worker nodes
  int maxnodes = m_conf->getconfi("processor", "nnodes");
  int idbase = m_conf->getconfi("processor", "idbase");
  char* hostbase = m_conf->getconf("processor", "nodebase");
  char* badlist = m_conf->getconf("processor", "badlist");

  char hostnode[512], idname[3];
  RFNSM_Status::Instance().set_flag(0);
  int nnodes = 0;
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    if (badlist == NULL  ||
        strstr(badlist, idname) == 0) {
      sprintf(hostnode, "evp_%s%2.2d", hostbase, idbase + i);
      //      b2nsm_sendreq(hostnode, "RF_STOP", 0, pars);
      b2nsm_sendreq(hostnode, "RC_START", 2, pars);
      nnodes++;
    }
  }

  return 0;
}

int RFMaster::Stop(NSMmsg* msg, NSMcontext*)
{
  int pars[10];
  pars[0] = msg->pars[0];
  pars[1] = msg->pars[1];

  // 1. Stop worker nodes
  // Unconfigure event processors
  int maxnodes = m_conf->getconfi("processor", "nnodes");
  int idbase = m_conf->getconfi("processor", "idbase");
  char* hostbase = m_conf->getconf("processor", "nodebase");
  char* badlist = m_conf->getconf("processor", "badlist");

  char hostnode[512], idname[3];
  RFNSM_Status::Instance().set_flag(0);
  int nnodes = 0;
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    if (badlist == NULL  ||
        strstr(badlist, idname) == 0) {
      sprintf(hostnode, "evp_%s%2.2d", hostbase, idbase + i);
      //      b2nsm_sendreq(hostnode, "RF_STOP", 0, pars);
      b2nsm_sendreq(hostnode, "RC_STOP", 0, pars);
      nnodes++;
    }
  }
#ifdef DESY
  b2nsm_wait(5);
#else
  while (RFNSM_Status::Instance().get_flag() != nnodes) b2nsm_wait(1);
#endif

  // 2. Stop DqmServer node
  // Unconfigure DqmServer
  char* dqmserver = m_conf->getconf("dqmserver", "nodename");
  RFNSM_Status::Instance().set_flag(0);
  //  b2nsm_sendreq(dqmserver, "RF_STOP", 0, pars);
  b2nsm_sendreq(dqmserver, "RC_STOP", 0, pars);
  while (RFNSM_Status::Instance().get_flag() == 0) b2nsm_wait(1);

  printf("RFMaster : Stopped. exp=%d, run=%d\n", pars[0], pars[1]);
  fflush(stdout);

  return 0;
}


int RFMaster::Restart(NSMmsg*, NSMcontext*)
{
  int* pars;

  // 0. Configure DqmServer
  char* dqmserver = m_conf->getconf("dqmserver", "nodename");
  //  b2nsm_sendreq(dqmserver, "RF_RESTART", 0, pars);
  b2nsm_sendreq(dqmserver, "RC_RECOVER", 0, pars);
  sleep(2);

  // 1. Configure distributor
  char* distributor = m_conf->getconf("distributor", "nodename");
  //  b2nsm_sendreq(distributor, "RF_RESTART", 0, pars);
  b2nsm_sendreq(distributor, "RC_RECOVER", 0, pars);
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
      //      b2nsm_sendreq(hostnode, "RF_RESTART", 0, pars);
      b2nsm_sendreq(hostnode, "RC_RECOVER", 0, pars);
    }
  }
  sleep(2);

  // 3. Configure collector
  char* collector = m_conf->getconf("collector", "nodename");
  //  b2nsm_sendreq(collector, "RF_RESTART", 0, pars);
  b2nsm_sendreq(collector, "RC_RECOVER", 0, pars);

  return 0;
}

// Server function

void RFMaster::monitor_loop()
{
  while (true) {
    b2nsm_wait(10);
  }
}



