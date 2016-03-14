//+
// File : RFDqmServer.cc
// Description : DQM server for RFARM to accumulate histograms
//               in a TMapFile
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 4 - Sep - 2013
//-

#include "daq/rfarm/manager/RFDqmServer.h"

using namespace Belle2;
using namespace std;

RFDqmServer::RFDqmServer(string conffile)
{
  // 0. Initialize configuration manager
  m_conf = new RFConf(conffile.c_str());
  char* nodename = m_conf->getconf("dqmserver", "nodename");
  //  char nodename[256];
  //  gethostname ( nodename, sizeof(nodename) );

  // 1. Set execution directory
  string execdir = string(m_conf->getconf("system", "execdir_base")) + "/dqmserver";

  mkdir(execdir.c_str(), 0755);
  chdir(execdir.c_str());

  // 2. Initialize process manager
  m_proc = new RFProcessManager(nodename);

  // 3. Initialize LogManager
  m_log = new RFLogManager(nodename);
}

RFDqmServer::~RFDqmServer()
{
  delete m_log;
  delete m_proc;
  delete m_conf;
}

// Functions hooked up by NSM2

int RFDqmServer::Configure(NSMmsg*, NSMcontext*)
{
  // 1. Run DqmSever
  char* dqmserver = m_conf->getconf("dqmserver", "script");
  char* port = m_conf->getconf("dqmserver", "port");
  char* mapfile = m_conf->getconf("dqmserver", "mapfile");

  m_pid_dqm = m_proc->Execute(dqmserver, port, mapfile);

  return 0;

}

int RFDqmServer::UnConfigure(NSMmsg*, NSMcontext*)
{
  //  system("killall hrelay hserver");
  if (m_pid_dqm != 0) {
    int status;
    kill(m_pid_dqm, SIGINT);
    waitpid(m_pid_dqm, &status, 0);
    printf("Unconfigre : done\n");
  }
  return 0;
}

int RFDqmServer::Start(NSMmsg*, NSMcontext*)
{
  return 0;
}

int RFDqmServer::Stop(NSMmsg*, NSMcontext*)
{
  return 0;
}


int RFDqmServer::Restart(NSMmsg*, NSMcontext*)
{
  printf("RFDqmServer : Restarted!!!!!\n");
  /* Original Impl
  if (m_pid_dqm != 0) {
    kill(m_pid_dqm, SIGINT);
  }
  system("killall hrelay hserver");
  fflush(stdout);
  */

  NSMmsg* nsmmsg = NULL;
  NSMcontext* nsmcontext = NULL;
  RFDqmServer::UnConfigure(nsmmsg, nsmcontext);
  sleep(2);
  RFDqmServer::Configure(nsmmsg, nsmcontext);
  return 0;
}

// Server function

void RFDqmServer::server()
{
  while (true) {
    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("RFDqmServer::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
  }
}

