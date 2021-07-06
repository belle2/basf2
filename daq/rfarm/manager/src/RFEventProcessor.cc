/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/rfarm/manager/RFEventProcessor.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#define RFOTSOUT stdout

//#define DESY

using namespace std;
using namespace Belle2;

RFEventProcessor::RFEventProcessor(string conffile)
{
  // 0. Initialize configuration manager
  m_conf = new RFConf(conffile.c_str());
  //  char* nodename = m_conf->getconf ( "processor", "nodename" );
  //  char nodename[256];
  strcpy(m_nodename, "evp_");
#ifndef DESY
  gethostname(&m_nodename[4], sizeof(m_nodename));
#else
  // Special treatment for DESY test nodes!!
  char hostnamebuf[256];
  gethostname(hostnamebuf, sizeof(hostnamebuf));
  strcat(&m_nodename[4], &hostnamebuf[6]);
  int lend = strlen(m_nodename);
  m_nodename[lend + 1] = (char)0;
  m_nodename[lend] = m_nodename[lend - 1];
  strncpy(&m_nodename[lend - 1], "0", 1);
  // End of DESY special treatment
#endif
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

  // 7. Clear PID list
  m_pid_sender = 0;
  m_pid_basf2 = 0;
  m_pid_receiver = 0;
  m_pid_hrecv = 0;
  m_pid_hrelay = 0;

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

int RFEventProcessor::Configure(NSMmsg* nsmm, NSMcontext* nsmc)
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

  // 3. Run sender / logger
  char* sender = m_conf->getconf("processor", "sender", "script");
  char* port = m_conf->getconf("processor", "sender", "port");
  m_pid_sender = m_proc->Execute(sender, (char*)rbufout.c_str(), port, (char*)shmname.c_str(), (char*)"1");
  m_flow->clear(1);

  /*
  // 4. Run basf2
  char* basf2 = m_conf->getconf("processor", "basf2", "script");
  if (nsmm->len > 0) {
    basf2 = (char*) nsmm->datap;
    printf("Configure: basf2 script overridden : %s\n", basf2);
  }
  m_pid_basf2 = m_proc->Execute(basf2, (char*)rbufin.c_str(), (char*)rbufout.c_str(), hport);
  */

  // 5. Run receiver
  char* receiver = m_conf->getconf("processor", "receiver", "script");
  char* srchost = m_conf->getconf("distributor", "host");
  //  char* port = m_conf->getconf ( "distributor", "port" );
  int portbase = m_conf->getconfi("distributor", "sender", "portbase");
  char* hostbase = m_conf->getconf("processor", "nodebase");
  int baselen = strlen(hostbase);
  /* OLD impl
  char hostname[256];
  gethostname(hostname, sizeof(hostname));
  char id[3];
  strcpy(id, &hostname[baselen + 1]);
  int rport = atoi(id) + portbase;
  */
  int rport;
  sscanf(&m_nodename[strlen(m_nodename) - 2], "%d", &rport);
  rport += portbase;
  char portchar[256];
  sprintf(portchar, "%d", rport);
  m_pid_receiver = m_proc->Execute(receiver, (char*)rbufin.c_str(), srchost, portchar, (char*)shmname.c_str(), (char*)"0");
  m_flow->clear(0);

  printf("Configure : done\n");
  fflush(stdout);

  // 6 Clear RingBuffers
  m_rbufin->forceClear();
  m_rbufout->forceClear();

  return 0;

}

int RFEventProcessor::UnConfigure(NSMmsg*, NSMcontext*)
{
  // Simple implementation to stop all processes
  //  system("killall basf2 sock2rbr rb2sockr hrelay hserver");

  // Emergency stop
  system("killall -9 python");

  // Normal abort
  int status;
  if (m_pid_sender != 0) {
    printf("RFEventProcessor : killing sender pid=%d\n", m_pid_sender);
    kill(m_pid_sender, SIGINT);
    waitpid(m_pid_sender, &status, 0);
  }
  if (m_pid_basf2 != 0) {
    printf("RFEventProcessor : killing basf2 pid=%d\n", m_pid_basf2);
    //    kill(m_pid_basf2, SIGINT);
    kill(m_pid_basf2, SIGINT);
    waitpid(m_pid_basf2, &status, 0);
    m_pid_basf2 = 0;
  }
  if (m_pid_receiver != 0) {
    printf("RFEventProcessor : killing receiver pid=%d\n", m_pid_receiver);
    kill(m_pid_receiver, SIGINT);
    waitpid(m_pid_receiver, &status, 0);
  }
  if (m_pid_hrecv != 0) {
    printf("RFEventProcessor : killing hserver pid=%d\n", m_pid_hrecv);
    kill(m_pid_hrecv, SIGINT);
    waitpid(m_pid_hrecv, &status, 0);
  }
  if (m_pid_hrelay != 0) {
    printf("RFEventProcessor : killing hrelay pid=%d\n", m_pid_hrelay);
    kill(m_pid_hrelay, SIGINT);
    waitpid(m_pid_hrelay, &status, 0);
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

int RFEventProcessor::Start(NSMmsg* nsmm, NSMcontext* nsmc)
{
  string rbufin = string(m_conf->getconf("system", "unitname")) + ":" +
                  string(m_conf->getconf("processor", "ringbufin"));
  string rbufout = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("processor", "ringbufout"));
  char* hport = m_conf->getconf("processor", "historecv", "port");

  // 0. Set run numbers
  m_expno = nsmm->pars[0];
  m_runno = nsmm->pars[1];

  // 1. Clear RingBuffers
  m_rbufout->forceClear();
  m_rbufin->forceClear();

  // 2. Run basf2
  char* basf2 = m_conf->getconf("processor", "basf2", "script");
  if (nsmm->len > 0) {
    basf2 = (char*) nsmm->datap;
    printf("Configure: basf2 script overridden : %s\n", basf2);
  }
  m_pid_basf2 = m_proc->Execute(basf2, (char*)rbufin.c_str(), (char*)rbufout.c_str(), hport);

  return 0;
}

int RFEventProcessor::Stop(NSMmsg* msgm, NSMcontext* msgc)
{
  printf("RFEventProcessor : STOP processing started\n");
  //  fflush ( stdout );
  /*
  char* hcollect = m_conf->getconf("processor", "dqm", "hcollect");
  char* filename = m_conf->getconf("processor", "dqm", "file");
  char* nprocs = m_conf->getconf("processor", "basf2", "nprocs");
  int pid_hcollect = m_proc->Execute(hcollect, filename, nprocs);
  int status;
  waitpid(pid_hcollect, &status, 0);
  */

  /*
  // Need to wait until all the events are processed and sent
  RfNodeInfo* nodeinfo = GetNodeInfo();
  int ncount = 0;
  for (;;) {
    RfShm_Cell& cellin = m_flow->getinfo(0);
    RfShm_Cell& cellout = m_flow->getinfo(1);
    if (cellin.nqueue == 0 && cellout.nqueue == 0) break;
    printf("inqueue = %d : outqueue = %d\n", cellin.nqueue, cellout.nqueue);
    if (ncount > 30) {
      printf("RFEventProcessor : Timeout (30sec) in stopping\n");
      break;
    }
    sleep(1);
    ncount ++;
  }
  */
  // Checking number of processed events using RfShm_Cell does not work as expected and is
  // equivalent to wait for 30 sec. Just replaced with 5 sec waiting.
  sleep(5);

  int status;
  if (m_pid_basf2 != 0) {
    printf("RFEventProcessor : killing basf2 pid=%d\n", m_pid_basf2);
    kill(m_pid_basf2, SIGINT);
    waitpid(m_pid_basf2, &status, 0);
    m_pid_basf2 = 0;
    char outfile[1024];
    sprintf(outfile, "dqm_e%4.4dr%6.6d.root", m_expno, m_runno);
    std::rename("histofile.root", outfile);
    printf("output file name = %s\n", outfile);
    fflush(stdout);

  }
  printf("RFEventProcessor : STOP processing done\n");
  return 0;
}


int RFEventProcessor::Restart(NSMmsg*, NSMcontext*)
{
  printf("RFEventProcessor : Restarting!!!!!\n");
  /* Original impl.
  if (m_pid_sender != 0) {
    printf("RFEventProcessor : killing sender pid=%d\n", m_pid_sender);
    kill(m_pid_sender, SIGINT);
  }
  if (m_pid_basf2 != 0) {
    printf("RFEventProcessor : killing basf2 pid=%d\n", m_pid_basf2);
    kill(m_pid_basf2, SIGINT);
  }
  if (m_pid_receiver != 0) {
    printf("RFEventProcessor : killing receiver pid=%d\n", m_pid_receiver);
    kill(m_pid_receiver, SIGINT);
  }
  if (m_pid_hrecv != 0) {
    printf("RFEventProcessor : killing hserver pid=%d\n", m_pid_hrecv);
    kill(m_pid_receiver, SIGINT);
  }
  if (m_pid_hrelay != 0) {
    printf("RFEventProcessor : killing hrelay pid=%d\n", m_pid_hrelay);
    kill(m_pid_receiver, SIGINT);
  }
  // Simple implementation to stop all processes
  system("killall basf2 sock2rbr rb2sockr hrelay hserver");
  fflush(stdout);
  */
  NSMmsg* nsmmsg = NULL;
  NSMcontext* nsmcontext = NULL;
  RFEventProcessor::UnConfigure(nsmmsg, nsmcontext);
  sleep(2);
  RFEventProcessor::Configure(nsmmsg, nsmcontext);
  return 0;
}

// Server function

void RFEventProcessor::server()
{
  // Clear PID list
  m_flow->fillProcessStatus(GetNodeInfo());
  // Start Loop
  while (true) {
    pid_t pid = m_proc->CheckProcess();
    if (pid > 0) {
      printf("RFEventProcessor : process dead pid=%d\n", pid);
      if (pid == m_pid_sender) {
        m_log->Fatal("RFEventProcessor : sender dead. pid=%d\n", m_pid_sender);
        m_pid_sender = 0;
      } else if (pid == m_pid_basf2) {
        m_log->Fatal("RFEventProcessor : basf2 dead. pid=%d\n", m_pid_basf2);
        m_pid_basf2 = 0;
      } else if (pid == m_pid_receiver) {
        m_log->Fatal("RFEventProcessor : receiver dead. pid=%d\n", m_pid_receiver);
        m_pid_receiver = 0;
      } else if (pid == m_pid_hrecv) {
        m_log->Fatal("RFEventProcessor : hserver dead. pid=%d\n", m_pid_hrecv);
        m_pid_hrecv = 0;
      } else if (pid == m_pid_hrelay) {
        m_log->Fatal("RFEventProcessor : hrelay dead. pid=%d\n", m_pid_hrelay);
        m_pid_hrelay = 0;
      }
    }
    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("RFEventProcessor::server");
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

void RFEventProcessor::cleanup()
{
  printf("RFEventProcessor : cleaning up\n");
  UnConfigure(NULL, NULL);
  printf("RFEventProcessor: Done. Exitting\n");
  exit(-1);
}



