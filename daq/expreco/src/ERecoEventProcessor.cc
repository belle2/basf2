//+
// File : ERecoEventProcessor.cc
// Description : Collect outputs from worker node and send them to EVB2
//               w/ branch to PXD
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
//-

#include "daq/expreco/ERecoEventProcessor.h"
#include <iostream>

#define RFOTSOUT stdout

using namespace std;
using namespace Belle2;

ERecoEventProcessor::ERecoEventProcessor(string conffile)
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
  string shmname = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_nodename);
  m_shm = new RFSharedMem((char*)shmname.c_str());

  // 3. Initialize process manager
  m_proc = new RFProcessManager(m_nodename);

  // 4. Initialize RingBuffers
  //  char* rbufin = m_conf->getconf("processor", "ringbufin");
  string rbufin = string(m_conf->getconf("system", "unitname")) + ":" +
                  string(m_conf->getconf("processor", "ringbufin"));
  int rbinsize = m_conf->getconfi("processor", "ringbufinsize");
  //  m_rbufin = new RingBuffer(rbufin, rbinsize);
  m_rbufin = new RingBuffer(rbufin.c_str(), rbinsize);
  //  char* rbufout = m_conf->getconf("processor", "ringbufout");
  string rbufout = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("processor", "ringbufout"));
  int rboutsize = m_conf->getconfi("processor", "ringbufoutsize");
  //  m_rbufout = new RingBuffer(rbufout, rboutsize);
  m_rbufout = new RingBuffer(rbufout.c_str(), rboutsize);

  // 5. Initialize LogManager
  m_log = new RFLogManager(m_nodename, m_conf->getconf("system", "lognode"));

  // 6. Initialize data flow monitor
  m_flow = new RFFlowStat((char*)shmname.c_str());

}

ERecoEventProcessor::~ERecoEventProcessor()
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

int ERecoEventProcessor::Configure(NSMmsg* nsmm, NSMcontext* nsmc)
{
  // Start processes from down stream

  // 0. Get common parameters
  //  char* rbufin = m_conf->getconf("processor", "ringbufin");
  string rbufin = string(m_conf->getconf("system", "unitname")) + ":" +
                  string(m_conf->getconf("processor", "ringbufin"));
  //  char* rbufout = m_conf->getconf("processor", "ringbufout");
  string rbufout = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("processor", "ringbufout"));

  string shmname = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_nodename);

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

  // 3. Run basf2
  /*
  char* basf2 = m_conf->getconf("processor", "basf2", "script");
  if (nsmm->len > 0) {
    basf2 = (char*) nsmm->datap;
    printf("Configure: basf2 script overridden : %s\n", basf2);
  }
  m_pid_basf2 = m_proc->Execute(basf2, (char*)rbufin.c_str(), (char*)rbufout.c_str(), hport);
  */

  // 4. Run receiver
  char* receiver = m_conf->getconf("processor", "receiver", "script");
  char* srchost = m_conf->getconf("distributor", "host");
  //  char* port = m_conf->getconf ( "distributor", "port" );
  int portbase = m_conf->getconfi("distributor", "sender", "portbase");
  char* hostbase = m_conf->getconf("processor", "nodebase");
  int baselen = strlen(hostbase);
  int rport;
  sscanf(&m_nodename[strlen(m_nodename) - 2], "%d", &rport);
  rport += portbase;
  char portchar[256];
  sprintf(portchar, "%d", rport);
  m_pid_receiver = m_proc->Execute(receiver, (char*)rbufin.c_str(), srchost, portchar, (char*)shmname.c_str(), (char*)"0");

  // 5. Run EventServer
  char* evs = m_conf->getconf("processor", "eventserver", "script");
  char* evsport = m_conf->getconf("processor", "eventserver", "port");
  m_pid_evs = m_proc->Execute(evs, (char*)rbufout.c_str(), evsport);

  printf("Configure : done\n");
  fflush(stdout);
  return 0;

}

int ERecoEventProcessor::UnConfigure(NSMmsg*, NSMcontext*)
{
  // Simple implementation to stop all processes
  //  system("killall basf2 sock2rbr rb2sockr hrelay hserver");
  int status;
  if (m_pid_sender != 0) {
    printf("ERecoEventProcessor : killing sender pid=%d\n", m_pid_sender);
    kill(m_pid_sender, SIGINT);
    waitpid(m_pid_sender, &status, 0);
  }
  if (m_pid_basf2 != 0) {
    printf("ERecoEventProcessor : killing basf2 pid=%d\n", m_pid_basf2);
    kill(m_pid_basf2, SIGINT);
    waitpid(m_pid_basf2, &status, 0);
  }
  if (m_pid_receiver != 0) {
    printf("ERecoEventProcessor : killing receiver pid=%d\n", m_pid_receiver);
    kill(m_pid_receiver, SIGINT);
    waitpid(m_pid_receiver, &status, 0);
  }
  if (m_pid_hrecv != 0) {
    printf("ERecoEventProcessor : killing hserver pid=%d\n", m_pid_hrecv);
    kill(m_pid_hrecv, SIGINT);
    waitpid(m_pid_hrecv, &status, 0);
  }
  if (m_pid_hrelay != 0) {
    printf("ERecoEventProcessor : killing hrelay pid=%d\n", m_pid_hrelay);
    kill(m_pid_hrelay, SIGINT);
    waitpid(m_pid_hrelay, &status, 0);
  }
  if (m_pid_evs != 0) {
    printf("ERecoEventProcessor : killing hrelay pid=%d\n", m_pid_hrelay);
    kill(m_pid_evs, SIGINT);
    waitpid(m_pid_evs, &status, 0);
  }

  // Clear RingBuffers
  m_rbufin->forceClear();
  m_rbufout->forceClear();

  // Clear PID list
  m_flow->fillProcessStatus(GetNodeInfo());

  printf("Unconfigure : done\n");
  fflush(stdout);
  return 0;
}

int ERecoEventProcessor::Start(NSMmsg* nsmm, NSMcontext* nsmc)
{
  string rbufin = string(m_conf->getconf("system", "unitname")) + ":" +
                  string(m_conf->getconf("processor", "ringbufin"));
  string rbufout = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("processor", "ringbufout"));
  char* hport = m_conf->getconf("processor", "historecv", "port");


  // 3. Run basf2
  char* basf2 = m_conf->getconf("processor", "basf2", "script");
  if (nsmm->len > 0) {
    basf2 = (char*) nsmm->datap;
    printf("Configure: basf2 script overridden : %s\n", basf2);
  }
  m_pid_basf2 = m_proc->Execute(basf2, (char*)rbufin.c_str(), (char*)rbufout.c_str(), hport);

  return 0;
}

int ERecoEventProcessor::Stop(NSMmsg*, NSMcontext*)
{
  /*
  char* hcollect = m_conf->getconf("processor", "dqm", "hcollect");
  char* filename = m_conf->getconf("processor", "dqm", "file");
  char* nprocs = m_conf->getconf("processor", "basf2", "nprocs");
  int pid_hcollect = m_proc->Execute(hcollect, filename, nprocs);
  int status;
  waitpid(pid_hcollect, &status, 0);
  */
  int status;
  if (m_pid_basf2 != 0) {
    printf("RFEventProcessor : killing basf2 pid=%d\n", m_pid_basf2);
    kill(m_pid_basf2, SIGINT);
    waitpid(m_pid_basf2, &status, 0);
    m_pid_basf2 = 0;
  }
  return 0;
}


int ERecoEventProcessor::Restart(NSMmsg*, NSMcontext*)
{
  printf("ERecoEventProcessor : Restarting!!!!!\n");
  /* Original impl.
  if (m_pid_sender != 0) {
    printf("ERecoEventProcessor : killing sender pid=%d\n", m_pid_sender);
    kill(m_pid_sender, SIGINT);
  }
  if (m_pid_basf2 != 0) {
    printf("ERecoEventProcessor : killing basf2 pid=%d\n", m_pid_basf2);
    kill(m_pid_basf2, SIGINT);
  }
  if (m_pid_receiver != 0) {
    printf("ERecoEventProcessor : killing receiver pid=%d\n", m_pid_receiver);
    kill(m_pid_receiver, SIGINT);
  }
  if (m_pid_hrecv != 0) {
    printf("ERecoEventProcessor : killing hserver pid=%d\n", m_pid_hrecv);
    kill(m_pid_receiver, SIGINT);
  }
  if (m_pid_hrelay != 0) {
    printf("ERecoEventProcessor : killing hrelay pid=%d\n", m_pid_hrelay);
    kill(m_pid_receiver, SIGINT);
  }
  // Simple implementation to stop all processes
  system("killall basf2 sock2rbr rb2sockr hrelay hserver");
  fflush(stdout);
  */
  NSMmsg* nsmmsg = NULL;
  NSMcontext* nsmcontext = NULL;
  ERecoEventProcessor::UnConfigure(nsmmsg, nsmcontext);
  sleep(2);
  ERecoEventProcessor::Configure(nsmmsg, nsmcontext);
  return 0;
}

// Server function

void ERecoEventProcessor::server()
{
  // Clear PID list
  m_flow->fillProcessStatus(GetNodeInfo());
  // Start Loop
  while (true) {
    pid_t pid = m_proc->CheckProcess();
    if (pid > 0) {
      printf("ERecoEventProcessor : process dead pid=%d\n", pid);
      if (pid == m_pid_sender) {
        m_log->Fatal("ERecoEventProcessor : sender dead. pid=%d\n", m_pid_sender);
        m_pid_sender = 0;
      } else if (pid == m_pid_basf2) {
        m_log->Fatal("ERecoEventProcessor : basf2 dead. pid=%d\n", m_pid_basf2);
        m_pid_basf2 = 0;
      } else if (pid == m_pid_receiver) {
        m_log->Fatal("ERecoEventProcessor : receiver dead. pid=%d\n", m_pid_receiver);
        m_pid_receiver = 0;
      } else if (pid == m_pid_hrecv) {
        m_log->Fatal("ERecoEventProcessor : hserver dead. pid=%d\n", m_pid_hrecv);
        m_pid_hrecv = 0;
      } else if (pid == m_pid_hrelay) {
        m_log->Fatal("ERecoEventProcessor : hrelay dead. pid=%d\n", m_pid_hrelay);
        m_pid_hrelay = 0;
      }
    }
    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("ERecoEventProcessor::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
    m_flow->fillNodeInfo(0, GetNodeInfo(), false);
    m_flow->fillNodeInfo(1, GetNodeInfo(), true);
    m_flow->fillProcessStatus(GetNodeInfo(), m_pid_receiver, m_pid_sender, m_pid_basf2,
                              m_pid_hrecv, m_pid_hrelay);
  }
}



