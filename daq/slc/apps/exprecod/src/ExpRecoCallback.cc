#include "daq/slc/apps/exprecod/ExpRecoCallback.h"

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

#include <sys/stat.h>

using namespace Belle2;
using namespace std;

static ExpRecoCallback* s_expreco = NULL;

//-----------------------------------------------------------------
// Rbuf-Read Thread Interface
//-----------------------------------------------------------------
void* RunExpRecoLogger(void*)
{
  s_expreco->ExpRecoLogger();
  return NULL;
}




ExpRecoCallback::ExpRecoCallback(const NSMNode& node)
  : RCCallback(node)
{
  // Conf file
  m_conf = new RFConf(getenv("EXPRECO_CONFFILE"));

  s_expreco = this;

}

ExpRecoCallback::~ExpRecoCallback() throw()
{

}

bool ExpRecoCallback::load() throw()
{
  // 1. Set execution directory
  //  char* chr_execdir = m_conf->getconf("expreco","execdir_base");
  //  printf ( "execdir = %s\n", chr_execdir );
  char* nodename = m_conf->getconf("expreco", "nodename");

  string execdir = string(m_conf->getconf("expreco", "execdir"));
  printf("execdir = %s\n", execdir.c_str());

  mkdir(execdir.c_str(), 0755);
  chdir(execdir.c_str());

  // 2. Initialize process manager
  m_proc = new RFProcessManager(nodename);

  // 3. Initialize log manager
  m_log = new RFLogManager(nodename);

  // 4. Initialize local shared memory
  m_shm = new RFSharedMem(nodename);

  // 5. Initialize RingBuffers
  char* rbinname = m_conf->getconf("expreco", "rbufin");
  int rbinsize = m_conf->getconfi("expreco", "rbufinsize");
  m_rbufin = new RingBuffer(rbinname, rbinsize);
  char* rboutname = m_conf->getconf("expreco", "rbufout");
  int rboutsize = m_conf->getconfi("expreco", "rbufoutsize");
  m_rbufout = new RingBuffer(rboutname, rboutsize);

  // 6. Start Logger
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  //  pthread_attr_setschedpolicy(&thread_attr , SCHED_FIFO);
  //  pthread_attr_setdetachstate(&thread_attr , PTHREAD_CREATE_DETACHED);
  //  pthread_t thr_input;
  pthread_create(&m_logthread, NULL, RunExpRecoLogger, NULL);

  // 7. Run basf2
  char* basf2 = m_conf->getconf("expreco", "basf2script");
  char* dqmdest = m_conf->getconf("dqmserver", "host");
  char* dqmport = m_conf->getconf("dqmserver", "port");
  m_pid_basf2 = m_proc->Execute(basf2, rbinname, dqmdest, dqmport, rboutname);

  // 8. Run receiver
  char* receiver = m_conf->getconf("expreco", "recvscript");
  char* srchost = m_conf->getconf("storage", "host");
  char* port = m_conf->getconf("storage", "port");
  m_pid_receiver = m_proc->Execute(receiver, rbinname, srchost, port, "expreco", (char*)"0");

  // 9. Run eventserver
  char* evs = m_conf->getconf("expreco", "evsscript");
  char* evsport = m_conf->getconf("expreco", "evsport");
  m_pid_evs = m_proc->Execute(evs, rboutname, evsport);

  return true;
}

bool ExpRecoCallback::start() throw()
{
  return true;
}

bool ExpRecoCallback::stop() throw()
{
  return true;
}

bool ExpRecoCallback::recover() throw()
{
  // Kill processes
  int status;
  if (m_pid_basf2 != 0) {
    kill(m_pid_basf2, SIGINT);
    waitpid(m_pid_basf2, &status, 0);
  }

  if (m_pid_receiver != 0) {
    kill(m_pid_receiver, SIGINT);
    waitpid(m_pid_receiver, &status, 0);
  }

  if (m_pid_evs != 0) {
    kill(m_pid_evs, SIGINT);
    waitpid(m_pid_evs, &status, 0);
  }

  // Clear RingBuffer
  m_rbufin->clear();
  m_rbufout->clear();

  // Do "load" again
  //  load();

  // Re-run killed processes
  char* rbinname = m_conf->getconf("expreco", "rbufin");
  char* rboutname = m_conf->getconf("expreco", "rbufout");

  char* basf2 = m_conf->getconf("expreco", "basf2script");
  char* dqmdest = m_conf->getconf("dqmserver", "host");
  char* dqmport = m_conf->getconf("dqmserver", "port");
  m_pid_basf2 = m_proc->Execute(basf2, rbinname, dqmdest, dqmport, rboutname);

  char* receiver = m_conf->getconf("expreco", "recvscript");
  char* srchost = m_conf->getconf("storage", "host");
  char* port = m_conf->getconf("storage", "port");
  m_pid_receiver = m_proc->Execute(receiver, rbinname, srchost, port, "expreco", (char*)"0");

  char* evs = m_conf->getconf("expreco", "evsscript");
  char* evsport = m_conf->getconf("expreco", "evsport");
  m_pid_evs = m_proc->Execute(evs, rboutname, evsport);

  return true;
}

void ExpRecoCallback::ExpRecoLogger()
{
  while (true) {
    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("ExpRecoLogger::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
  }
}



