//+
// File : ERecoEventSampler.cc
// Description : Receive events from Storage and distribute them to
//                 processing nodes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 31 - Jul - 2017
//-

#include "daq/expreco/ERecoEventSampler.h"

#include <iostream>

#define RFEVSOUT stdout

using namespace std;
using namespace Belle2;

ERecoEventSampler::ERecoEventSampler(string conffile)
{

  // 0. Initialize configuration manager
  m_conffile = conffile;
  m_conf = new RFConf(conffile.c_str());
  char* nodename = m_conf->getconf("eventsampler", "nodename");

  // 1. Set execution directory
  string execdir = string(m_conf->getconf("system", "execdir_base")) + "/sampler";

  mkdir(execdir.c_str(), 0755);
  chdir(execdir.c_str());

  // 2. Initialize RingBuffers
  char* rbufout = m_conf->getconf("eventsampler", "ringbufout");
  int rbufsize = m_conf->getconfi("eventsampler", "ringbufoutsize");
  m_rbufout = new RingBuffer(rbufout, rbufsize);

  // 3. Initialize process manager
  m_proc = new RFProcessManager(nodename);

  // 4. Initialize LogManager
  m_log = new RFLogManager(nodename, m_conf->getconf("system", "lognode"));

  m_pid_sampler = 0;

}

ERecoEventSampler::~ERecoEventSampler()
{
  delete m_log;
  delete m_proc;
  delete m_conf;
  delete m_rbufout;
}


// Functions hooked up by NSM2

int ERecoEventSampler::Configure(NSMmsg*, NSMcontext*)
{

  /*
  // 1. Run EventSampler
  char* sampler = m_conf->getconf("eventsampler", "script");
  m_pid_sampler = m_proc->Execute(sampler, (char*)m_conffile.c_str());
  */

  /* Public event server moved outside
  // 2. Run EventServer
  char* server = m_conf->getconf("eventsampler", "server", "script");
  char* rbuf = m_conf->getconf("eventsampler", "ringbufout");
  char* port = m_conf->getconf("eventsampler", "server", "port");
  m_pid_server = m_proc->Execute(server, rbuf, port);
  */

  printf("ERecoEventSampler : Configure done\n");
  return 0;
}

int ERecoEventSampler::UnConfigure(NSMmsg*, NSMcontext*)
{
  /*
  int status;
  printf("ERecoEventSampler: Unconfigure pids = %d %d\n", m_pid_sampler, m_pid_server);
  fflush(stdout);
  if (m_pid_sampler != 0) {
    kill(m_pid_sampler, SIGINT);
    waitpid(m_pid_sampler, &status, 0);
    m_pid_sampler = 0;
  }
  */
  /*
  if (m_pid_server != 0) {
    kill(m_pid_server, SIGINT);
    waitpid(m_pid_server, &status, 0);
    m_pid_server = 0;
  }
  */
  printf("ERecoEventSampler : Unconfigure done\n");
  return 0;
}

int ERecoEventSampler::Start(NSMmsg*, NSMcontext*)
{
  // 1. Run EventSampler
  char* sampler = m_conf->getconf("eventsampler", "script");
  m_pid_sampler = m_proc->Execute(sampler, (char*)m_conffile.c_str());

  /* Public event server moved outside
  // 2. Run EventServer
  char* server = m_conf->getconf("eventsampler", "server", "script");
  char* rbuf = m_conf->getconf("eventsampler", "ringbufout");
  char* port = m_conf->getconf("eventsampler", "server", "port");
  m_pid_server = m_proc->Execute(server, rbuf, port);
  */

  printf("ERecoEventSampler : Configure done\n");


  //  m_rbufin->clear();
  return 0;
}

int ERecoEventSampler::Stop(NSMmsg*, NSMcontext*)
{
  int status;
  printf("ERecoEventSampler: Unconfigure pids = %d %d\n", m_pid_sampler, m_pid_server);
  fflush(stdout);
  if (m_pid_sampler != 0) {
    kill(m_pid_sampler, SIGINT);
    waitpid(m_pid_sampler, &status, 0);
    m_pid_sampler = 0;
  }

//  m_rbufin->clear();
  return 0;
}


int ERecoEventSampler::Restart(NSMmsg*, NSMcontext*)
{
}

// Server function

void ERecoEventSampler::server()
{
  while (true) {
    pid_t pid = m_proc->CheckProcess();
    if (pid > 0) {
      if (pid == m_pid_sampler)
        printf("ERecoEventSampler : eventsampler process dead. pid = %d\n", pid);
      else
        printf("ERecoEventSampler : unknown process dead. pid = %d\n", pid);
    }

    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("ERecoEventSampler::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
  }
}




