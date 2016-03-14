//+
// File : RFEventSever.cc
// Description : Receive events from EVB1 and distribute them to
//                 processing nodes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
//-

#include "daq/rfarm/manager/RFEventServer.h"

#define RFEVSOUT stdout

using namespace std;
using namespace Belle2;

RFEventServer* RFEventServer::s_instance = 0;
//RFServerBase* RFServerBase::s_instance = 0;

RFEventServer::RFEventServer(string conffile)
{
  // 0. Initialize configuration manager
  m_conf = new RFConf(conffile.c_str());
  char* nodename = m_conf->getconf("distributor", "nodename");
  //  char nodename[256];
  //  gethostname ( nodename, sizeof(nodename) );

  // 1. Set execution directory
  string execdir = string(m_conf->getconf("system", "execdir_base")) + "/distributor";

  mkdir(execdir.c_str(), 0755);
  chdir(execdir.c_str());

  // 2. Initialize local shared memory
  m_shm = new RFSharedMem(nodename);

  // 3. Initialize process manager
  m_proc = new RFProcessManager(nodename);

  // 4. Initialize RingBuffers
  char* ringbuf = m_conf->getconf("distributor", "ringbuffer");
  int rbufsize = m_conf->getconfi("distributor", "ringbuffersize");
  m_rbufin = new RingBuffer(ringbuf, rbufsize);

  // 5. Initialize LogManager
  m_log = new RFLogManager(nodename);

  // 6. Initialize data flow monitor
  m_flow = new RFFlowStat(nodename);

}

RFEventServer::~RFEventServer()
{
  delete m_log;
  delete m_proc;
  delete m_shm;
  delete m_conf;
  delete m_flow;
  delete m_rbufin;
}

// Access to Singleton
RFEventServer& RFEventServer::Create(string conffile)
{
  if (!s_instance) {
    s_instance = new RFEventServer(conffile);
  }
  return *s_instance;
}

RFEventServer& RFEventServer::Instance()
{
  return *s_instance;
}


// Functions hooked up by NSM2

int RFEventServer::Configure(NSMmsg*, NSMcontext*)
{
  // 0. Global parameters
  char* ringbuf = m_conf->getconf("distributor", "ringbuffer");

  // 2. Run sender
  m_nnodes = 0;
  int maxnodes = m_conf->getconfi("processor", "nnodes");
  int idbase = m_conf->getconfi("processor", "idbase");
  char* hostbase = m_conf->getconf("processor", "hostbase");
  char* badlist = m_conf->getconf("processor", "badlist");

  char* sender = m_conf->getconf("distributor", "sender", "script");
  int portbase = m_conf->getconfi("distributor", "sender", "portbase");

  char* shmname = m_conf->getconf("distributor", "nodename");

  char hostname[512], idname[3], shmid[3];
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    sprintf(shmid, "%2.2d", i);
    if (badlist == NULL  ||
        strstr(badlist, idname) == 0) {
      int port = (idbase + i) + portbase;
      char portchar[256];
      sprintf(portchar, "%d", port);
      m_pid_sender[m_nnodes] = m_proc->Execute(sender, ringbuf, portchar, shmname, shmid);
      m_nnodes++;
    }
  }

  // 1. Run receiver
  char* src = m_conf->getconf("distributor", "source");
  if (strstr(src, "net") != 0) {
    // Run receiver
    char* receiver = m_conf->getconf("distributor", "receiver", "script");
    char* src = m_conf->getconf("distributor", "receiver", "host");
    char* port = m_conf->getconf("distributor", "receiver", "port");
    char* ringbuf = m_conf->getconf("distributor", "ringbuffer");
    char idbuf[3];
    sprintf(idbuf, "%2.2d", RF_INPUT_ID);
    m_pid_recv = m_proc->Execute(receiver, ringbuf, src, port, shmname, idbuf);
  } else if (strstr(src, "file") != 0) {
    // Run file reader
    char* filein = m_conf->getconf("distributor", "fileinput", "script");
    char* file = m_conf->getconf("distributor", "fileinput", "filename");
    char* ringbuf = m_conf->getconf("distributor", "ringbuffer");
    char* nnodechr = m_conf->getconf("distributor", "nnodes");
    m_pid_recv = m_proc->Execute(filein, ringbuf, file, nnodechr);
  }
  // else none
  return 0;
}

int RFEventServer::UnConfigure(NSMmsg*, NSMcontext*)
{
  //  system("killall sock2rbr rb2sockr");
  int status;
  if (m_pid_recv != 0) {
    kill(m_pid_recv, SIGINT);
    waitpid(m_pid_recv, &status, 0);
  }
  for (int i = 0; i < m_nnodes; i++) {
    if (m_pid_sender[i] != 0) {
      printf("RFEventServer:: killing sender pid=%d\n", m_pid_sender[i]);
      kill(m_pid_sender[i], SIGINT);
      waitpid(m_pid_sender[i], &status, 0);
    }
  }

  printf("Unconfigure : done\n");
  return 0;
}

int RFEventServer::Start(NSMmsg*, NSMcontext*)
{
  m_rbufin->clear();
  return 0;
}

int RFEventServer::Stop(NSMmsg*, NSMcontext*)
{
  m_rbufin->clear();
  return 0;
}


int RFEventServer::Restart(NSMmsg*, NSMcontext*)
{
  printf("RFEventServer : Restarting!!!!!\n");
  /* Original impl.
  if (m_pid_recv != 0) {
    kill(m_pid_recv, SIGINT);
  }
  for (int i = 0; i < m_nnodes; i++) {
    if (m_pid_sender[i] != 0) {
      printf("RFEventServer:: killing sender pid=%d\n", m_pid_sender[i]);
      kill(m_pid_sender[i], SIGINT);
    }
  }
  // Simple implementation
  system("killall sock2rbr rb2sockr");
  fflush(stdout);
  */
  NSMmsg* nsmmsg = NULL;
  NSMcontext* nsmcontext = NULL;
  RFEventServer::UnConfigure(nsmmsg, nsmcontext);
  sleep(2);
  RFEventServer::Configure(nsmmsg, nsmcontext);
  return 0;
}

// Server function

void RFEventServer::server()
{
  while (true) {
    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("RFEventServer::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
    m_flow->fillNodeInfo(RF_INPUT_ID, GetNodeInfo(), false);
  }
}



