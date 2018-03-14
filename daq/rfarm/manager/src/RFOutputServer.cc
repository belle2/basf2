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
  string shmname = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(nodename);
  m_shm = new RFSharedMem((char*)shmname.c_str());

  // 3. Initialize RingBuffers
  //  char* rbufin = m_conf->getconf("collector", "ringbufin");
  string rbufin = string(m_conf->getconf("system", "unitname")) + ":" +
                  string(m_conf->getconf("collector", "ringbufin"));
  int rbinsize = m_conf->getconfi("collector", "ringbufinsize");
  m_rbufin = new RingBuffer(rbufin.c_str(), rbinsize);
  //  char* rbufout = m_conf->getconf("collector", "ringbufout");
  string rbufout = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("collector", "ringbufout"));
  int rboutsize = m_conf->getconfi("collector", "ringbufoutsize");
  m_rbufout = new RingBuffer(rbufout.c_str(), rboutsize);

  // 4. Initialize process manager
  m_proc = new RFProcessManager(nodename);

  // 5. Initialize LogManager
  m_log = new RFLogManager(nodename, m_conf->getconf("system", "lognode"));

  // 6. Initialize data flow monitor
  m_flow = new RFFlowStat((char*)shmname.c_str());

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

int RFOutputServer::Configure(NSMmsg* nsmm, NSMcontext* nsmc)
{
  // Start processes from down stream

  // 0. Get common parameters
  //  char* rbufin = m_conf->getconf("collector", "ringbufin");
  string rbufin = string(m_conf->getconf("system", "unitname")) + ":" +
                  string(m_conf->getconf("collector", "ringbufin"));
  //  char* rbufout = m_conf->getconf("collector", "ringbufout");
  string rbufout = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("collector", "ringbufout"));

  //  char* shmname = m_conf->getconf("collector", "nodename");
  string shmname = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("collector", "nodename"));

  // 1. Histogram Receiver
  char* hrecv = m_conf->getconf("collector", "historecv", "script");
  char* hport = m_conf->getconf("collector", "historecv", "port");
  char* mapfile = m_conf->getconf("collector", "historecv", "mapfile");
  //  m_pid_hrecv = m_proc->Execute(hrecv, hport, mapfile);

  // 2. Histogram Relay
  char* hrelay = m_conf->getconf("collector", "historelay", "script");
  char* dqmdest = m_conf->getconf("dqmserver", "host");
  char* dqmport = m_conf->getconf("dqmserver", "port");
  char* interval = m_conf->getconf("collector", "historelay", "interval");
  //  m_pid_hrelay = m_proc->Execute(hrelay, mapfile, dqmdest, dqmport, interval);

  // 3. Run sender / logger
  char* src = m_conf->getconf("collector", "destination");
  if (strstr(src, "net") != 0) {
    // Run sender
    char* sender = m_conf->getconf("collector", "sender", "script");
    char* port = m_conf->getconf("collector", "sender", "port");
    char idbuf[3];
    sprintf(idbuf, "%2.2d", RF_OUTPUT_ID);
    m_pid_sender = m_proc->Execute(sender, (char*)rbufout.c_str(), port, (char*)shmname.c_str(), idbuf);
    m_flow->clear(RF_OUTPUT_ID);
  } else if (strstr(src, "file") != 0) {
    // Run file writer
    char* writer = m_conf->getconf("collector", "writer", "script");
    char* file = m_conf->getconf("collector", "writer", "filename");
    char* nnode = m_conf->getconf("processor", "nnodes");
    m_pid_sender = m_proc->Execute(writer, (char*)rbufout.c_str(), file, nnode);
  } else {
    // Do not run anything
  }

  // 4. Run basf2
  char* basf2 = m_conf->getconf("collector", "basf2", "script");
  if (nsmm->len > 0) {
    basf2 = (char*) nsmm->datap;
    printf("Configure: basf2 script overridden : %s\n", basf2);
  }
  m_pid_basf2 = m_proc->Execute(basf2, (char*)rbufin.c_str(), (char*)rbufout.c_str(), hport);

  // 5. Run receiver
  m_nnodes = 0;
  int maxnodes = m_conf->getconfi("processor", "nnodes");
  int idbase = m_conf->getconfi("processor", "idbase");
  //  char* hostbase = m_conf->getconf("processor", "hostbase");
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
      m_pid_receiver[m_nnodes] = m_proc->Execute(receiver, (char*)rbufin.c_str(), hostname, port, (char*)shmname.c_str(), shmid);
      m_flow->clear(i);
      m_nnodes++;
    }
  }
  return 0;
}

int RFOutputServer::UnConfigure(NSMmsg*, NSMcontext*)
{
  //  system("killall sock2rbr rb2sockr basf2 hrelay hserver");

  printf("m_pid_sender = %d\n", m_pid_sender);
  printf("m_pid_basf2 = %d\n", m_pid_basf2);
  fflush(stdout);
  int status, ws;
  if (m_pid_sender != 0) {
    printf("killing sender %d\n", m_pid_sender);
    //    kill(m_pid_sender, SIGINT);
    kill(m_pid_sender, SIGKILL);
    ws = waitpid(m_pid_sender, &status, 0);
    printf("wait return = %d, status = %d\n", ws, status);
  }

  if (m_pid_basf2 != 0) {
    printf("killing sender %d\n", m_pid_sender);
    kill(m_pid_basf2, SIGINT);
    ws = waitpid(m_pid_basf2, &status, 0);
    printf("wait return = %d, status = %d\n", ws, status);
  }

  for (int i = 0; i < m_nnodes; i++) {
    if (m_pid_receiver[i] != 0) {
      printf("killing receiver %d\n", m_pid_receiver[i]);
      kill(m_pid_receiver[i], SIGINT);
      ws = waitpid(m_pid_receiver[i], &status, 0);
      printf("wait return = %d, status = %d\n", ws, status);
    }
  }

  // Clear RingBuffer
  m_rbufin->forceClear();
  m_rbufout->forceClear();

  // Clear process list
  m_flow->fillProcessStatus(GetNodeInfo());

  printf("Unconfigure done\n");
  fflush(stdout);
  return 0;
}

int RFOutputServer::Start(NSMmsg*, NSMcontext*)
{
  // Clear RingBuffer
  //  m_rbufin->forceClear();
  //  m_rbufout->forceClear();
  return 0;
}

int RFOutputServer::Stop(NSMmsg*, NSMcontext*)
{
  return 0;
}


int RFOutputServer::Restart(NSMmsg*, NSMcontext*)
{
  printf("RFOutputServer : Restarting!!!!!!\n");
  /* Original Impl
  kill(m_pid_sender, SIGINT);
  kill(m_pid_basf2, SIGINT);
  for (int i = 0; i < m_nnodes; i++) {
    kill(m_pid_receiver[i], SIGINT);
  }
  // Simple Implementation
  system("killall sock2rbr rb2sockr basf2 hrelay hserver");
  fflush(stdout);
  */
  NSMmsg* nsmmsg = NULL;
  NSMcontext* nsmcontext = NULL;
  RFOutputServer::UnConfigure(nsmmsg, nsmcontext);
  sleep(2);
  RFOutputServer::Configure(nsmmsg, nsmcontext);
  return 0;
}

// Server function

void RFOutputServer::server()
{
  m_flow->fillProcessStatus(GetNodeInfo());
  while (true) {
    int recv_id = 0;
    pid_t pid = m_proc->CheckProcess();
    if (pid > 0) {
      printf("RFOutputServer : process dead. pid=%d\n", pid);
      if (pid == m_pid_sender) {
        m_log->Fatal("RFOutputServer : sender process dead. pid=%d\n", pid);
        m_pid_sender = 0;
      } else if (pid == m_pid_basf2) {
        m_log->Fatal("RFOutputServer : basf2 process dead. pid=%d\n", pid);
        m_pid_basf2 = 0;
      } else {
        for (int i = 0; i < m_nnodes; i++) {
          if (pid == m_pid_receiver[i]) {
            m_log->Fatal("RFOutputServer : receiver process %d dead. pid=%d\n", i, pid);
            m_pid_receiver[i] = 0;
            recv_id = i;
          }
        }
      }
    }

    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("RFOutputServer::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
    m_flow->fillNodeInfo(RF_OUTPUT_ID, GetNodeInfo(), true);
    m_flow->fillProcessStatus(GetNodeInfo(), m_pid_receiver[recv_id], m_pid_sender,
                              m_pid_basf2);
  }
}



