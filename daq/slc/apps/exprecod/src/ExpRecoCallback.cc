#include "daq/slc/apps/exprecod/ExpRecoCallback.h"

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/State.h>

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




ExpRecoCallback::ExpRecoCallback(NSMNode* node)
  : RCCallback(node)
{
  node->setData(new DataObject());
  node->setState(State::INITIAL_S);

  // Conf file
  m_conf = new RFConf(getenv("EXPRECO_CONFFILE"));

  s_expreco = this;

}

ExpRecoCallback::~ExpRecoCallback() throw()
{

}

bool ExpRecoCallback::boot() throw()
{
  Belle2::debug("BOOT");

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
  char* rbufname = m_conf->getconf("expreco", "ringbufin");
  int rbinsize = m_conf->getconfi("expreco", "ringbufsize");
  m_rbuf = new RingBuffer(rbufname, rbinsize);

  // 6. Start Logger
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  //  pthread_attr_setschedpolicy(&thread_attr , SCHED_FIFO);
  //  pthread_attr_setdetachstate(&thread_attr , PTHREAD_CREATE_DETACHED);
  //  pthread_t thr_input;
  pthread_create(&m_logthread, NULL, RunExpRecoLogger, NULL);

  // 7. Run basf2
  char* basf2 = m_conf->getconf("expreco", "basf2script");
  char* rbufin = m_conf->getconf("expreco", "ringbufin");
  char* dqmdest = m_conf->getconf("dqmserver", "host");
  char* dqmport = m_conf->getconf("dqmserver", "port");
  m_pid_basf2 = m_proc->Execute(basf2, rbufin, dqmdest, dqmport);

  // 8. Run receiver
  char* receiver = m_conf->getconf("expreco", "recvscript");
  char* srchost = m_conf->getconf("storage", "host");
  char* port = m_conf->getconf("storage", "port");
  m_pid_receiver = m_proc->Execute(receiver, rbufin, srchost, port, "expreco", (char*)"0");

  // 9. Run eventserver
  char* evs = m_conf->getconf("expreco", "evsscript");
  char* evsport = m_conf->getconf("expreco", "evsport");
  m_pid_evs = m_proc->Execute(evs, evsport);

  debug("BOOT done");

  return true;
}

bool ExpRecoCallback::load() throw()
{
  Belle2::debug("LOAD");
  //  download();
  //  _node->getData()->print();


  return true;
}

bool ExpRecoCallback::start() throw()
{
  Belle2::debug("START");
  Belle2::debug("Exp no: %d", (int)getMessage().getParam(0));
  Belle2::debug("Run no: %d", (int)getMessage().getParam(1));
  return true;
}

bool ExpRecoCallback::stop() throw()
{
  Belle2::debug("STOP");
  return true;
}

bool ExpRecoCallback::recover() throw()
{
  Belle2::debug("RECOVER");

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
  m_rbuf->clear();

  // Do "load" again
  load();

  return true;
}

bool ExpRecoCallback::trigft() throw()
{
  Belle2::debug("TRIGFT");
  Belle2::debug("trigger_mode  : %d", (int)getMessage().getParam(0));
  Belle2::debug("dummy_rate    : %d", (int)getMessage().getParam(1));
  Belle2::debug("trigger_limit : %d", (int)getMessage().getParam(2));
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



