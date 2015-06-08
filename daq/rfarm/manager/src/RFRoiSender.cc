//+
// File : RFRoiSender.cc
// Description : DQM server for RFARM to accumulate histograms
//               in a TMapFile
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 4 - Sep - 2013
//-

#include "daq/rfarm/manager/RFRoiSender.h"

using namespace Belle2;
using namespace std;

RFRoiSender::RFRoiSender(string conffile)
{
  // 0. Initialize configuration manager
  m_conf = new RFConf(conffile.c_str());
  char* nodename = m_conf->getconf("roisender", "nodename");
  //  char nodename[256];
  //  gethostname ( nodename, sizeof(nodename) );

  // 1. Set execution directory
  string execdir = string(m_conf->getconf("system", "execdir_base")) + "/roisender";

  mkdir(execdir.c_str(), 0755);
  chdir(execdir.c_str());

  // 2. Initialize process manager
  m_proc = new RFProcessManager(nodename);

  // 3. Initialize LogManager
  m_log = new RFLogManager(nodename);
}

RFRoiSender::~RFRoiSender()
{
  delete m_log;
  delete m_proc;
  delete m_conf;
}

// Functions hooked up by NSM2

int RFRoiSender::Configure(NSMmsg*, NSMcontext*)
{
  // 0. Do you need RoI sender?
  char* roisw = m_conf->getconf("roisender", "enabled");
  if (strstr(roisw, "yes") == 0) return 0;

  // 1. Run merger first
  char* merger = m_conf->getconf("roisender", "merger");
  char* mergerport = m_conf->getconf("roisender", "mergerport");
  char* mergerhost = m_conf->getconf("roisender", "mergerhost");
  char* onsenhost = m_conf->getconf("roisender", "onsenhost");
  char* onsenport = m_conf->getconf("roisender", "onsenport");

  // Note: merger should be run on a separate host in Belle2DAQ
  if (strstr(merger, "none") == 0) {
    m_pid_merger = m_proc->Execute(merger, onsenhost, onsenport, mergerport);
    sleep(2);
  }

  // 2. Run sender
  char* sender = m_conf->getconf("roisender", "sender");
  int nqueue = m_conf->getconfi("roisender", "nqueues");
  char* qbase = m_conf->getconf("roisender", "qnamebase");

  char* arglist[20];
  arglist[0] = mergerhost;
  arglist[1] = mergerport;
  int nargs = 2;
  char roiqs[10][256];
  for (int i = 0; i < nqueue; i++) {
    sprintf(roiqs[i], "/roi%d", i);
    arglist[i + 2] = roiqs[i];
    nargs++;
  }
  //  m_pid_sender = m_proc->Execute(sender, mergerhost, mergerport);
  m_pid_sender = m_proc->Execute(sender, nargs, arglist);

  return 0;

}

int RFRoiSender::UnConfigure(NSMmsg*, NSMcontext*)
{
  //  system("killall merger_merge hltout2merger");
  int statx;
  if (m_pid_sender != 0) {
    kill(m_pid_sender, SIGINT);
    waitpid(m_pid_sender, &statx, 0);
  }
  if (m_pid_merger != 0) {
    kill(m_pid_merger, SIGINT);
    waitpid(m_pid_merger, &statx, 0);
  }
  printf("UnConfigure : done\n");
  return 0;
}

int RFRoiSender::Start(NSMmsg*, NSMcontext*)
{
  return 0;
}

int RFRoiSender::Stop(NSMmsg*, NSMcontext*)
{
  return 0;
}


int RFRoiSender::Restart(NSMmsg*, NSMcontext*)
{
  printf("RFRoiSender : Restarted!!!!!\n");
  /* Original Impl
  if (m_pid_dqm != 0) {
    kill(m_pid_dqm, SIGINT);
  }
  */
  //  system("killall merger_merge hltout2merger");
  //  fflush(stdout);
  NSMmsg* nsmmsg = NULL;
  NSMcontext* nsmcontext = NULL;
  RFRoiSender::UnConfigure(nsmmsg, nsmcontext);
  sleep(2);
  RFRoiSender::Configure(nsmmsg, nsmcontext);
  return 0;
}

// Server function

void RFRoiSender::server()
{
  while (true) {
    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("RFRoiSender::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
  }
}

