#include "daq/expreco/EventServerCallback.h"

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

#include <sys/stat.h>

using namespace Belle2;
using namespace std;

static EventServerCallback* s_eventserver = NULL;

//-----------------------------------------------------------------
// Rbuf-Read Thread Interface
//-----------------------------------------------------------------
void* RunEventServerLogger(void*)
{
  s_eventserver->EventServerLogger();
  return NULL;
}




EventServerCallback::EventServerCallback()
{
  // Conf file

  // 0. Initialize configuration manager
  m_conf = new RFConf(getenv("ERECO_CONFFILE"));

  s_eventserver = this;
}

EventServerCallback::~EventServerCallback() throw()
{

}

void EventServerCallback::load(const DBObject&) throw(RCHandlerException)
{

  // 1. Set execution directory
  char* nodename = m_conf->getconf("eventsampler", "server", "nodename");
  string execdir = string(m_conf->getconf("system", "execdir_base")) + "/eventserver";
  printf("execdir = %s\n", execdir.c_str());

  mkdir(execdir.c_str(), 0755);
  chdir(execdir.c_str());

  // 2. Initialize process manager
  m_proc = new RFProcessManager(nodename);

  // 3. Initialize log manager
  m_log = new RFLogManager(nodename);


  // 4. EventServer
  char* server = m_conf->getconf("eventsampler", "server", "script");
  char* rbuf = m_conf->getconf("eventsampler", "ringbufout");
  char* port = m_conf->getconf("eventsampler", "server", "port");
  m_pid_server = m_proc->Execute(server, rbuf, port);

  sleep(2);

  // 6. Start Logger
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  //  pthread_attr_setschedpolicy(&thread_attr , SCHED_FIFO);
  //  pthread_attr_setdetachstate(&thread_attr , PTHREAD_CREATE_DETACHED);
  //  pthread_t thr_input;
  pthread_create(&m_logthread, NULL, RunEventServerLogger, NULL);

}

void EventServerCallback::start() throw(RCHandlerException)
{
}

void EventServerCallback::stop() throw(RCHandlerException)
{
}

void EventServerCallback::abort() throw(RCHandlerException)
{
  // Kill processes
  int status;
  if (m_pid_server != 0) {
    kill(m_pid_server, SIGINT);
    waitpid(m_pid_server, &status, 0);
    LogFile::info("killed event server (pid=%d)", m_pid_server);
  }

  pthread_cancel(m_logthread);

}

void EventServerCallback::recover(const DBObject&) throw(RCHandlerException)
{

}

void EventServerCallback::EventServerLogger()
{
  while (true) {
    int st = m_proc->CheckOutput();
    if (st < 0) {
      perror("EventServerLogger::server");
      //      exit ( -1 );
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
  }
}



