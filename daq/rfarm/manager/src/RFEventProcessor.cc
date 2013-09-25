//+
// File : RFEventProcessor.cc
// Description : Collect outputs from worker node and send them to EVB2
//               w/ branch to PXD
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
//-

#include "daq/rfarm/manager/RFEventProcessor.h"

#define RFOTSOUT stdout

using namespace std;
using namespace Belle2;

RFEventProcessor::RFEventProcessor(string conffile)
{
  // 0. Initialize configuration manager
  m_conf = new RFConf(conffile.c_str());
  //  char* nodename = m_conf->getconf ( "processor", "nodename" );
  //  char nodename[256];
  strcpy(m_nodename, "evp_");
  gethostname(&m_nodename[4], sizeof(m_nodename));
  printf("nodename = %s\n", m_nodename);

  // 1. Set execution directory
  string execdir = string(m_conf->getconf("system", "execdir_base")) + "/" + string(m_nodename);
  printf("execdir = %s\n", execdir.c_str());

  mkdir(execdir.c_str(), 0755);
  chdir(execdir.c_str());

  // 2. Initialize local shared memory
  m_shm = new RFSharedMem(m_nodename);

  // 3. Initialize process manager
  m_proc = new RFProcessManager(m_nodename);

  // 4. Initialize RingBuffers
  char* rbufin = m_conf->getconf("processor", "ringbufin");
  int rbinsize = m_conf->getconfi("processor", "ringbufinsize");
  m_rbufin = new RingBuffer(rbufin, rbinsize);
  char* rbufout = m_conf->getconf("processor", "ringbufout");
  int rboutsize = m_conf->getconfi("processor", "ringbufoutsize");
  m_rbufout = new RingBuffer(rbufout, rboutsize);

  // 5. Initialize LogManager
  m_log = new RFLogManager(m_nodename);

  // 6. Initialize data flow monitor
  m_flow = new RFFlowStat(m_nodename);

}

RFEventProcessor::~RFEventProcessor()
{
  delete m_log;
  delete m_proc;
  delete m_shm;
  delete m_conf;
  delete m_flow;
  delete m_rbufin;
  delete m_rbufout;
}


// Functions hooked up by NSM2

void RFEventProcessor::Configure(NSMmsg*, NSMcontext*)
{
  // Start processes from down stream

  // 0. Get common parameters
  char* rbufin = m_conf->getconf("processor", "ringbufin");
  char* rbufout = m_conf->getconf("processor", "ringbufout");

  // 1. Histogram Receiver
  char* hrecv = m_conf->getconf("processor", "historecv", "script");
  char* hport = m_conf->getconf("processor", "historecv", "port");
  char* mapfile = m_conf->getconf("processor", "historecv", "mapfile");
  m_pid_hrecv = m_proc->Execute(hrecv, hport, mapfile);

  sleep(5);    // make sure that TMapFile is created.

  // 2. Histogram Relay
  char* hrelay = m_conf->getconf("processor", "historelay", "script");
  char* dqmdest = m_conf->getconf("dqmserver", "host");
  char* dqmport = m_conf->getconf("dqmserver", "port");
  char* interval = m_conf->getconf("processor", "historelay", "interval");
  m_pid_hrelay = m_proc->Execute(hrelay, mapfile, dqmdest, dqmport, interval);

  // 3. Run sender / logger
  char* sender = m_conf->getconf("processor", "sender", "script");
  char* port = m_conf->getconf("processor", "sender", "port");
  m_pid_sender = m_proc->Execute(sender, rbufout, port, m_nodename, (char*)"1");

  // 4. Run basf2
  char* basf2 = m_conf->getconf("processor", "basf2", "script");
  m_pid_basf2 = m_proc->Execute(basf2, rbufin, rbufout, hport);

  // 5. Run receiver
  char* receiver = m_conf->getconf("processor", "receiver", "script");
  char* srchost = m_conf->getconf("distributor", "host");
  //  char* port = m_conf->getconf ( "distributor", "port" );
  int portbase = m_conf->getconfi("distributor", "sender", "portbase");
  char* hostbase = m_conf->getconf("processor", "nodebase");
  int baselen = strlen(hostbase);
  char hostname[256];
  gethostname(hostname, sizeof(hostname));
  char id[3];
  strcpy(id, &hostname[baselen + 1]);
  int rport = atoi(id) + portbase;
  char portchar[256];
  sprintf(portchar, "%d", rport);
  m_pid_receiver = m_proc->Execute(receiver, rbufin, srchost, portchar, m_nodename, (char*)"0");

}

void RFEventProcessor::Start(NSMmsg*, NSMcontext*)
{
}

void RFEventProcessor::Stop(NSMmsg*, NSMcontext*)
{
}


void RFEventProcessor::Restart(NSMmsg*, NSMcontext*)
{
  if (m_pid_sender != 0) {
    printf("RFEventProcessor : killing sender pid=%d\n", m_pid_sender);
    kill(m_pid_sender, SIGINT);
  }
  if (m_pid_basf2 != 0) {
    printf("RFEventProcessor : killing basf2 pid=%d\n", m_pid_basf2);
    kill(m_pid_basf2, SIGQUIT);
  }
  if (m_pid_receiver != 0) {
    printf("RFEventProcessor : killing receiver pid=%d\n", m_pid_receiver);
    kill(m_pid_receiver, SIGINT);
  }
  sleep(2);
  NSMmsg* nsmmsg = NULL;
  NSMcontext* nsmcontext = NULL;
  RFEventProcessor::Configure(nsmmsg, nsmcontext);
}

// Server function

void RFEventProcessor::server()
{
  while (true) {
    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("RFEventProcessor::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
    m_flow->fillNodeInfo(0, GetNodeInfo(), false);
    m_flow->fillNodeInfo(1, GetNodeInfo(), true);
  }
}



