//+
// File : RFOutputSever.cc
// Description : Collect outputs from worker node and send them to EVB2
//               w/ branch to PXD
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
//-

#include "daq/rfarm/manager/RFOutputServer.h"

#define RFOTSOUT stdout

using namespace std;
using namespace Belle2;

RFOutputServer::RFOutputServer(string conffile)
{
  // 0. Initialize configuration manager
  m_conf = new RFConf(conffile.c_str());
  char* nodename = m_conf->getconf("collector", "nodename");
  //  char nodename[256];
  //  gethostname(nodename, sizeof(nodename));

  // 1. Set execution directory
  string execdir = string(m_conf->getconf("system", "execdir_base")) + "/collector";

  mkdir(execdir.c_str(), 0755);
  chdir(execdir.c_str());

  // 2. Initialize local shared memory
  m_shm = new RFSharedMem(nodename);

  // 3. Initialize RingBuffers
  char* rbufin = m_conf->getconf("collector", "ringbufin");
  int rbinsize = m_conf->getconfi("collector", "ringbufinsize");
  m_rbufin = new RingBuffer(rbufin, rbinsize);
  char* rbufout = m_conf->getconf("collector", "ringbufout");
  int rboutsize = m_conf->getconfi("collector", "ringbufoutsize");
  m_rbufout = new RingBuffer(rbufout, rboutsize);

  // 4. Initialize process manager
  m_proc = new RFProcessManager(nodename);

  // 5. Initialize LogManager
  m_log = new RFLogManager(nodename);

  // 6. Initialize data flow monitor
  m_flow = new RFFlowStat(nodename);

}

RFOutputServer::~RFOutputServer()
{
  delete m_log;
  delete m_proc;
  delete m_shm;
  delete m_conf;
  delete m_rbufin;
  delete m_rbufout;
  delete m_flow;
}


// Functions hooked up by NSM2

void RFOutputServer::Configure(NSMmsg*, NSMcontext*)
{
  // Start processes from down stream

  // 0. Get common parameters
  char* rbufin = m_conf->getconf("collector", "ringbufin");
  char* rbufout = m_conf->getconf("collector", "ringbufout");

  char* shmname = m_conf->getconf("collector", "nodename");

  // 1. Run sender / logger
  char* src = m_conf->getconf("collector", "destination");
  if (strstr(src, "net") != 0) {
    // Run sender
    char* sender = m_conf->getconf("collector", "sender", "script");
    char* port = m_conf->getconf("collector", "sender", "port");
    char idbuf[3];
    sprintf(idbuf, "%2.2d", RF_OUTPUT_ID);
    m_pid_sender = m_proc->Execute(sender, rbufout, port, shmname, idbuf);
  } else if (strstr(src, "file") != 0) {
    // Run file writer
    char* writer = m_conf->getconf("collector", "writer", "script");
    char* file = m_conf->getconf("collector", "writer", "filename");
    char* nnode = m_conf->getconf("processor", "nnodes");
    m_pid_sender = m_proc->Execute(writer, rbufout, file, nnode);
  } else {
    // Do not run anything
  }

  // 2. Run basf2
  char* basf2 = m_conf->getconf("collector", "basf2", "script");
  m_pid_basf2 = m_proc->Execute(basf2, rbufin, rbufout);

  // 3. Run receiver
  m_nnodes = 0;
  int maxnodes = m_conf->getconfi("processor", "nnodes");
  int idbase = m_conf->getconfi("processor", "idbase");
  char* hostbase = m_conf->getconf("processor", "hostbase");
  char* badlist = m_conf->getconf("processor", "badlist");
  char* port = m_conf->getconf("processor", "sender", "port");

  char* receiver = m_conf->getconf("collector", "receiver", "script");
  char hostname[512], idname[3], shmid[3];
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    sprintf(shmid, "%2.2d", i);
    if (badlist == NULL  ||
        strstr(badlist, idname) == 0) {
      sprintf(hostname, "%s%2.2d", hostbase, idbase + i);
      m_pid_receiver[m_nnodes] = m_proc->Execute(receiver, rbufin, hostname, port, shmname, shmid);
      m_nnodes++;
    }
  }

  // 4. Run histo receiver
  // 4. Histogram Receiver
  char* hrecv = m_conf->getconf("collector", "historecv", "script");
  char* hport = m_conf->getconf("collector", "historecv", "port");
  char* mapfile = m_conf->getconf("collector", "historecv", "mapfile");
  m_pid_hrecv = m_proc->Execute(hrecv, hport, mapfile);

  // 5. Histogram Relay
  char* hrelay = m_conf->getconf("collector", "historelay", "script");
  char* dqmdest = m_conf->getconf("dqmserver", "host");
  char* dqmport = m_conf->getconf("dqmserver", "port");
  char* interval = m_conf->getconf("collector", "historelay", "interval");
  m_pid_hrelay = m_proc->Execute(hrelay, mapfile, dqmdest, dqmport, interval);
}

void RFOutputServer::Start(NSMmsg*, NSMcontext*)
{
}

void RFOutputServer::Stop(NSMmsg*, NSMcontext*)
{
}


void RFOutputServer::Restart(NSMmsg*, NSMcontext*)
{
  kill(m_pid_sender, SIGTERM);
  kill(m_pid_basf2, SIGTERM);
  for (int i = 0; i < m_nnodes; i++) {
    kill(m_pid_receiver[i], SIGTERM);
  }
  sleep(2);
  NSMmsg* nsmmsg = NULL;
  NSMcontext* nsmcontext = NULL;
  RFOutputServer::Configure(nsmmsg, nsmcontext);
}

// Server function

void RFOutputServer::server()
{
  while (true) {
    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("RFOutputServer::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
  }
}



