//+
// File : ERecoDistributor.cc
// Description : Receive events from Storage and distribute them to
//                 processing nodes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
//-

#include "daq/expreco/ERecoDistributor.h"

#include <iostream>

#define RFEVSOUT stdout

using namespace std;
using namespace Belle2;

ERecoDistributor::ERecoDistributor(string conffile)
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
  string shmname = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("distributor", "nodename"));
  m_shm = new RFSharedMem((char*)shmname.c_str());

  // 3. Initialize process manager
  m_proc = new RFProcessManager(nodename);

  // 4. Initialize RingBuffers
  //  char* ringbuf = m_conf->getconf("distributor", "ringbuffer");
  string ringbuf = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("distributor", "ringbuffer"));
  int rbufsize = m_conf->getconfi("distributor", "ringbuffersize");
  m_rbufin = new RingBuffer(ringbuf.c_str(), rbufsize);

  // 5. Initialize LogManager
  m_log = new RFLogManager(nodename, m_conf->getconf("system", "lognode"));

  // 6. Initialize data flow monitor
  m_flow = new RFFlowStat((char*)shmname.c_str());

}

ERecoDistributor::~ERecoDistributor()
{
  delete m_log;
  delete m_proc;
  delete m_shm;
  delete m_conf;
  delete m_flow;
  delete m_rbufin;
}


// Functions hooked up by NSM2

int ERecoDistributor::Configure(NSMmsg*, NSMcontext*)
{
  // 0. Global parameters
  //  char* ringbuf = m_conf->getconf("distributor", "ringbuffer");
  string ringbuf = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("distributor", "ringbuffer"));

  //  char* shmname = m_conf->getconf("distributor", "nodename");
  string shmname = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("distributor", "nodename"));

  // 1. Run sender
  m_nnodes = 0;
  int maxnodes = m_conf->getconfi("processor", "nnodes");
  int idbase = m_conf->getconfi("processor", "idbase");
  char* hostbase = m_conf->getconf("processor", "hostbase");
  char* badlist = m_conf->getconf("processor", "badlist");

  char* sender = m_conf->getconf("distributor", "sender", "script");
  int portbase = m_conf->getconfi("distributor", "sender", "portbase");


  char hostname[512], idname[3], shmid[3];
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    sprintf(shmid, "%2.2d", i);
    if (badlist == NULL  ||
        strstr(badlist, idname) == 0) {
      int port = (idbase + i) + portbase;
      char portchar[256];
      sprintf(portchar, "%d", port);
      m_pid_sender[m_nnodes] = m_proc->Execute(sender, (char*)ringbuf.c_str(), portchar, (char*)shmname.c_str(), shmid);
      printf("Running sender to %d\n", port);
      fflush(stdout);
      m_nnodes++;
    }
  }

  // 2. Run receiver
  m_nrecv = 0;
  int maxrecv = m_conf->getconfi("distributor", "receiver", "nnodes");
  int ridbase = m_conf->getconfi("distributor", "receiver", "idbase");
  char* rhostbase = m_conf->getconf("distributor", "receiver", "hostbase");
  char* rbadlist = m_conf->getconf("distributor", "receiver", "badlist");
  char* port = m_conf->getconf("distributor", "receiver", "port");
  char* receiver = m_conf->getconf("distributor", "receiver", "script");

  //  char hostname[512], idname[3], shmid[3];
  for (int i = 0; i < maxrecv; i++) {
    sprintf(hostname, "%s%2.2d", rhostbase, ridbase + i);
    sprintf(idname, "%2.2d", ridbase + i);
    sprintf(shmid, "%2.2d", i + 20);
    if (rbadlist == NULL  ||
        strstr(rbadlist, hostname) == 0) {
      printf("Running receiver for %s\n", hostname);
      fflush(stdout);
      m_pid_recv[m_nrecv] = m_proc->Execute(receiver, (char*)ringbuf.c_str(), hostname, port, (char*)shmname.c_str(), shmid);
      m_nrecv++;
    }
  }

  printf("ERecoDistributor : Configure done\n");
  return 0;
}

int ERecoDistributor::UnConfigure(NSMmsg*, NSMcontext*)
{
  //  system("killall sock2rbr rb2sockr");
  int status;

  printf("m_nrecv = %d\n", m_nrecv);
  for (int i = 0; i < m_nrecv; i++) {
    if (m_pid_recv[i] != 0) {
      printf("ERecoDistributor : killing receiver pid=%d\n", m_pid_sender[i]);
      kill(m_pid_recv[i], SIGKILL);
      waitpid(m_pid_recv[i], &status, 0);
    }
  }

  printf("m_nnodes = %d\n", m_nnodes);
  for (int i = 0; i < m_nnodes; i++) {
    if (m_pid_sender[i] != 0) {
      printf("ERecoDistributor : killing sender pid=%d\n", m_pid_sender[i]);
      //      kill(m_pid_sender[i], SIGINT);
      kill(m_pid_sender[i], SIGKILL);
      waitpid(m_pid_sender[i], &status, 0);
    }
  }
  // Clear RingBuffer
  m_rbufin->forceClear();

  // Clear process list
  m_flow->fillProcessStatus(GetNodeInfo());

  printf("ERecoDistributor : Unconfigure done\n");
  return 0;
}

int ERecoDistributor::Start(NSMmsg*, NSMcontext*)
{
  //  m_rbufin->clear();
  return 0;
}

int ERecoDistributor::Stop(NSMmsg*, NSMcontext*)
{
  //  m_rbufin->clear();
  return 0;
}


int ERecoDistributor::Restart(NSMmsg*, NSMcontext*)
{
  printf("ERecoDistributor : Restarting!!!!!\n");
  NSMmsg* nsmmsg = NULL;
  NSMcontext* nsmcontext = NULL;
  ERecoDistributor::UnConfigure(nsmmsg, nsmcontext);
  sleep(2);
  ERecoDistributor::Configure(nsmmsg, nsmcontext);
  return 0;
}

// Server function

void ERecoDistributor::server()
{
  //  int nevt = 0;
  m_flow->fillProcessStatus(GetNodeInfo());

  while (true) {
    int sender_id = 0;
    pid_t pid = m_proc->CheckProcess();
    if (pid > 0) {
      printf("ERecoDistributor : process dead. pid = %d\n", pid);
      for (int i = 0; i < m_nrecv; i++) {
        if (pid == m_pid_recv[i]) {
          m_log->Fatal("ERecoDistributor : receiver process (%d) dead. pid=%d\n", i, pid);
          m_pid_recv[i] = 0;
        }
      }
    } else {
      for (int i = 0; i < m_nnodes; i++) {
        if (pid == m_pid_sender[i]) {
          m_log->Fatal("ERecoDistributor : sender process (%d) dead. pid=%d\n", i, m_pid_sender[i]);
          m_pid_sender[i] = 0;
          sender_id = i;
        }
      }
    }

    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("ERecoDistributor::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
  }
  //    m_flow->fillNodeInfo(RF_INPUT_ID, GetNodeInfo(), false);
  //    m_flow->fillProcessStatus(GetNodeInfo(), m_pid_recv, m_pid_sender[sender_id]);
  //    m_flow->fillNodeInfo(0, GetNodeInfo(), false);
  // Debug
  //    RfNodeInfo* info = GetNodeInfo();
  //info->nevent_in = nevt++;
  //    info->nqueue_in = nevt;
  //    printf ( "FillNodeInfo called!! info->nevent_in = %d\n", info->nevent_in );
}




