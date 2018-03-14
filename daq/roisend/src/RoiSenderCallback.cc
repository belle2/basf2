#include "daq/roisend/RoiSenderCallback.h"

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

#include <sys/stat.h>

using namespace Belle2;
using namespace std;

static RoiSenderCallback* s_roisender = NULL;

//-----------------------------------------------------------------
// Rbuf-Read Thread Interface
//-----------------------------------------------------------------
void* RunRoiSenderLogger(void*)
{
  s_roisender->RoiSenderLogger();
  return NULL;
}




RoiSenderCallback::RoiSenderCallback()
{
  // Conf file
  m_conf = new RFConf(getenv("ROISENDER_CONFFILE"));

  s_roisender = this;
}

RoiSenderCallback::~RoiSenderCallback() throw()
{

}

void RoiSenderCallback::load(const DBObject&) throw(RCHandlerException)
{
  // 1. Set execution directory
  //  char* chr_execdir = m_conf->getconf("expreco","execdir_base");
  //  printf ( "execdir = %s\n", chr_execdir );
  char* nodename = m_conf->getconf("roisender", "nodename");

  string execdir = string(m_conf->getconf("roisender", "execdir"));
  printf("execdir = %s\n", execdir.c_str());

  mkdir(execdir.c_str(), 0755);
  chdir(execdir.c_str());

  // 2. Initialize process manager
  m_proc = new RFProcessManager(nodename);

  // 3. Initialize log manager
  m_log = new RFLogManager(nodename);

  // 4. Initialize local shared memory
  m_shm = new RFSharedMem(nodename);


  // 5. Run MergerMerge
  char* merger = m_conf->getconf("roisender", "merger");
  char* mergerport = m_conf->getconf("roisender", "mergerport");
  char* mergerhost = m_conf->getconf("roisender", "mergerhost");
  char* onsenhost = m_conf->getconf("roisender", "onsenhost");
  char* onsenport = m_conf->getconf("roisender", "onsenport");

  string shmname = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("roisender", "nodename"));

  char idbuf[3];
  sprintf(idbuf, "%2.2d", 0);
  m_pid_merger = m_proc->Execute(merger, (char*)shmname.c_str(), idbuf, onsenhost, onsenport, mergerport);
  sleep(2);

  // 6. Start Logger
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  //  pthread_attr_setschedpolicy(&thread_attr , SCHED_FIFO);
  //  pthread_attr_setdetachstate(&thread_attr , PTHREAD_CREATE_DETACHED);
  //  pthread_t thr_input;
  pthread_create(&m_logthread, NULL, RunRoiSenderLogger, NULL);

}

void RoiSenderCallback::start() throw(RCHandlerException)
{
}

void RoiSenderCallback::stop() throw(RCHandlerException)
{
}

void RoiSenderCallback::abort() throw(RCHandlerException)
{
  // Kill processes
  int status;
  if (m_pid_merger != 0) {
    kill(m_pid_merger, SIGINT);
    waitpid(m_pid_merger, &status, 0);
    LogFile::info("killd merger (pid=%d)", m_pid_merger);
  }

  pthread_cancel(m_logthread);

}

void RoiSenderCallback::recover(const DBObject&) throw(RCHandlerException)
{
  // Kill processes
  int status;
  if (m_pid_merger != 0) {
    kill(m_pid_merger, SIGINT);
    waitpid(m_pid_merger, &status, 0);
    LogFile::info("killd merger (pid=%d)", m_pid_merger);
  }

  // 1. Run merger first
  char* merger = m_conf->getconf("roisender", "merger");
  char* mergerport = m_conf->getconf("roisender", "mergerport");
  char* mergerhost = m_conf->getconf("roisender", "mergerhost");
  char* onsenhost = m_conf->getconf("roisender", "onsenhost");
  char* onsenport = m_conf->getconf("roisender", "onsenport");

  string shmname = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("roisender", "nodename"));

  char idbuf[3];
  sprintf(idbuf, "%2.2d", 0);
  m_pid_merger = m_proc->Execute(merger, (char*)shmname.c_str(), idbuf, onsenhost, onsenport, mergerport);
  sleep(2);

}

void RoiSenderCallback::RoiSenderLogger()
{
  while (true) {
    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("RoiSenderLogger::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
  }
}



