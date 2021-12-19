/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/rfarm/manager/RFEventServer.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <csignal>
#include <cstring>
#include <iostream>

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

  // 7. Clear PID list
  m_pid_recv = 0;
  for (int i = 0; i < m_nnodes; i++)
    m_pid_sender[i] = 0 ;

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
  //  char* ringbuf = m_conf->getconf("distributor", "ringbuffer");
  string ringbuf = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("distributor", "ringbuffer"));

  //  char* shmname = m_conf->getconf("distributor", "nodename");
  string shmname = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("distributor", "nodename"));

  // 2. Run sender
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
      m_flow->clear(i);
      m_nnodes++;
    }
  }

  // 1. Run receiver
  char* srcG = m_conf->getconf("distributor", "source");
  if (strstr(srcG, "net") != 0) {
    // Run receiver
    char* receiver = m_conf->getconf("distributor", "receiver", "script");
    char* src = m_conf->getconf("distributor", "receiver", "host");
    char* port = m_conf->getconf("distributor", "receiver", "port");
    //    char* ringbuf = m_conf->getconf("distributor", "ringbuffer");
    char idbuf[3];
    sprintf(idbuf, "%2.2d", RF_INPUT_ID);
    m_pid_recv = m_proc->Execute(receiver, (char*)ringbuf.c_str(), src, port, (char*)shmname.c_str(), idbuf);
    m_flow->clear(RF_INPUT_ID);
  } else if (strstr(srcG, "file") != 0) {
    // Run file reader
    char* filein = m_conf->getconf("distributor", "fileinput", "script");
    char* file = m_conf->getconf("distributor", "fileinput", "filename");
    //    char* ringbuf = m_conf->getconf("distributor", "ringbuffer");
    char* nnodechr = m_conf->getconf("distributor", "nnodes");
    m_pid_recv = m_proc->Execute(filein, (char*)ringbuf.c_str(), file, nnodechr);
  }

  m_rbufin->forceClear();

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
      //      kill(m_pid_sender[i], SIGINT);
      kill(m_pid_sender[i], SIGINT);
      waitpid(m_pid_sender[i], &status, 0);
    }
  }
  // Clear RingBuffer
  m_rbufin->forceClear();

  // Clear process list
  m_flow->fillProcessStatus(GetNodeInfo());

  printf("Unconfigure : done\n");
  return 0;
}

int RFEventServer::Start(NSMmsg*, NSMcontext*)
{
  //  m_rbufin->clear();
  //  m_rbufin->forceClear();
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
  //  int nevt = 0;
  m_flow->fillProcessStatus(GetNodeInfo());

  while (true) {
    int sender_id = 0;
    pid_t pid = m_proc->CheckProcess();
    if (pid > 0) {
      printf("RFEventServer : process dead. pid = %d\n", pid);
      if (pid == m_pid_recv) {
        m_log->Fatal("RFEventServer : receiver process dead. pid=%d\n", pid);
        m_pid_recv = 0;
      } else {
        for (int i = 0; i < m_nnodes; i++) {
          if (pid == m_pid_sender[i]) {
            m_log->Fatal("RFEventServer : sender process (%d) dead. pid=%d\n", i, m_pid_sender[i]);
            m_pid_sender[i] = 0;
            sender_id = i;
          }
        }
      }
    }
    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("RFEventServer::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
    m_flow->fillNodeInfo(RF_INPUT_ID, GetNodeInfo(), false);
    m_flow->fillProcessStatus(GetNodeInfo(), m_pid_recv, m_pid_sender[sender_id]);
    //    m_flow->fillNodeInfo(0, GetNodeInfo(), false);
    // Debug
    //    RfNodeInfo* info = GetNodeInfo();
    //info->nevent_in = nevt++;
    //    info->nqueue_in = nevt;
    //    printf ( "FillNodeInfo called!! info->nevent_in = %d\n", info->nevent_in );
  }
}

void RFEventServer::cleanup()
{
  printf("RFEventServer : cleaning up\n");
  UnConfigure(NULL, NULL);
  /*
  kill ( m_pid_recv, SIGINT );
  int status;
  waitpid ( m_pid_recv, &status, 0 );
  printf ( "RFEventServer : receiver terminated.\n" );
  for ( int i=0; i<m_nnodes; i++ ) {
    kill ( m_pid_sender[i], SIGINT );
    waitpid ( m_pid_sender[i], &status, 0 );
    printf ( "RFEventServer : sender [%d] terminated.\n", i );
  }
  */
  printf("RFEventServer: Done. Exitting\n");
  exit(-1);
}

