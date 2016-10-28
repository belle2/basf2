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
  m_log = new RFLogManager(nodename, m_conf->getconf("system", "lognode"));
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

  // 2. Run Dqm Histogram Relay
  char* relayhost = m_conf->getconf("dqmserver", "historelay", "host");
  char* relayport = m_conf->getconf("dqmserver", "historelay", "port");
  char* hrelay = m_conf->getconf("dqmserver", "historelay", "script");
  char* interval = m_conf->getconf("dqmserver", "historelay", "interval");

  if (strcmp(relayhost, "none") != 0) {
    printf("DqmServer : command = %s %s %s %s %s\n",
           hrelay, mapfile, relayhost, relayport, interval);
    m_pid_relay = m_proc->Execute(hrelay, mapfile, relayhost, relayport,
                                  interval);
  }

  return 0;

}

int RFDqmServer::UnConfigure(NSMmsg*, NSMcontext*)
{
  //  system("killall hrelay hserver");
  if (m_pid_dqm != 0) {
    int status;
    kill(m_pid_dqm, SIGINT);
    waitpid(m_pid_dqm, &status, 0);
  }
  if (m_pid_relay != 0) {
    int status;
    kill(m_pid_relay, SIGINT);
    waitpid(m_pid_relay, &status, 0);
  }
  printf("Unconfigure : done\n");
  return 0;
}

int RFDqmServer::Start(NSMmsg*, NSMcontext*)
{
  return 0;
}

int RFDqmServer::Stop(NSMmsg* msg, NSMcontext*)
{
  char* merger = m_conf->getconf("dqmserver", "merge", "script");
  char* topdir = m_conf->getconf("system", "exec_dir_base");
  char* infile = m_conf->getconf("processor", "dqm", "file");
  char* nnodes = m_conf->getconf("processor", "nnodes");
  char* startnode = m_conf->getconf("processor", "idbase");
  char outfile[1024];
  sprintf(outfile, "dqm_%e%4.4dr%6.6d.root", msg->pars[0], msg->pars[1]);

  int pid_dqmmerge = m_proc->Execute(merger, topdir, infile, nnodes, startnode, outfile);

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

