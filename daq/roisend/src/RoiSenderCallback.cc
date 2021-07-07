/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/roisend/RoiSenderCallback.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <csignal>

using namespace Belle2;
using namespace std;

static RoiSenderCallback* s_roisender = NULL;

//-----------------------------------------------------------------
// Rbuf-Read Thread Interface
//-----------------------------------------------------------------
void* RunRoiSenderLogger(void*)
{
  s_roisender->server();
  return NULL;
}




RoiSenderCallback::RoiSenderCallback()
{
  // Conf file
  m_conf = new RFConf(getenv("ROISENDER_CONFFILE"));

  s_roisender = this;
}


void RoiSenderCallback::load(const DBObject&, const std::string&)
{
  // 1. Set execution directory
  //  char* chr_execdir = m_conf->getconf("expreco","execdir_base");
  //  printf ( "execdir = %s\n", chr_execdir );
  char* nodename = m_conf->getconf("roisender", "nodename");

  string execdir = string(m_conf->getconf("roisender", "execdir"));
  printf("execdir = %s\n", execdir.c_str());

  mkdir(execdir.c_str(), 0755);
  chdir(execdir.c_str());

  // 2. Initialize process manager if not existing
  if (!m_proc) m_proc = new RFProcessManager(nodename);

  // 3. Initialize log manager if not existing
  if (!m_log) m_log = new RFLogManager(nodename, nodename);

  // 4. Initialize local shared memory if not existing, this seems to be used by flow monitoring in mergemerge?
  if (!m_shm) m_shm = new RFSharedMem(nodename);


  // 5. Run MergerMerge
  char* merger = m_conf->getconf("roisender", "merger");
  char* mergerport = m_conf->getconf("roisender", "mergerport");
//   char* mergerhost = m_conf->getconf("roisender", "mergerhost");
  char* onsenhost = m_conf->getconf("roisender", "onsenhost");
  char* onsenport = m_conf->getconf("roisender", "onsenport");

  string shmname = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("roisender", "nodename"));

  char idbuf[11]; // maximum length by cppcheck
  sprintf(idbuf, "%2.2d", 0);
  m_pid_merger = m_proc->Execute(merger, (char*)shmname.c_str(), idbuf, onsenhost, onsenport, mergerport);
  sleep(2); // WHY a sleep? if it is a race condition find & destroy!

  // 6. Start Logger -- only on first load
  if (!m_logthread) {
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    //  pthread_attr_setschedpolicy(&thread_attr , SCHED_FIFO);
    //  pthread_attr_setdetachstate(&thread_attr , PTHREAD_CREATE_DETACHED);
    //  pthread_t thr_input;
    pthread_create(&m_logthread, NULL, RunRoiSenderLogger, NULL);
  }
}

void RoiSenderCallback::start(int /*expno*/, int /*runno*/)
{
  if (m_pid_merger != 0) {
    int pid = m_pid_merger;
    kill(pid, SIGUSR1);
    LogFile::info("Send SIGUSR1 to (pid=%d)", pid);// attention, race condition!
  }
}

void RoiSenderCallback::stop(void)
{
  if (m_pid_merger != 0) {
    int pid = m_pid_merger;
    kill(pid, SIGUSR2);
    LogFile::info("Send SIGUSR2 to (pid=%d)", pid);// attention, race condition!
  }
}

void RoiSenderCallback::abort(void)
{
  // Kill processes
  if (m_pid_merger != 0) {
    int pid = m_pid_merger;
    kill(pid, SIGINT);
    LogFile::info("kill merger (pid=%d) with SIGINT ", pid);// attention, race condition!
  }
  // wait until
  for (int i = 0; m_pid_merger; i++) {
    sleep(1);
    int pid = m_pid_merger;
    if (i == 5) {
      kill(pid, SIGKILL); // force!
      LogFile::warning("kill merger (pid=%d) with SIGKILL", pid);
    }
    if (i == 10) {
      LogFile::error("killing merger (pid=%d) did not work", pid);
      m_pid_merger = 0;
      break;
    }
  }
}

void RoiSenderCallback::recover(const DBObject&, const std::string&)
{
  // Kill processes
  abort();

  sleep(2); // WHY a sleep? if it is a race condition find & destroy!

  // 1. Run merger first
  char* merger = m_conf->getconf("roisender", "merger");
  char* mergerport = m_conf->getconf("roisender", "mergerport");
//   char* mergerhost = m_conf->getconf("roisender", "mergerhost");
  char* onsenhost = m_conf->getconf("roisender", "onsenhost");
  char* onsenport = m_conf->getconf("roisender", "onsenport");

  string shmname = string(m_conf->getconf("system", "unitname")) + ":" +
                   string(m_conf->getconf("roisender", "nodename"));

  char idbuf[11];// maxlength by cppcheck
  sprintf(idbuf, "%2.2d", 0);// why this?
  m_pid_merger = m_proc->Execute(merger, (char*)shmname.c_str(), idbuf, onsenhost, onsenport, mergerport);
  sleep(2); // WHY a sleep? if it is a race condition find & destroy!
}

void RoiSenderCallback::server()
{
  while (true) {
    pid_t pid = m_proc->CheckProcess();
    // pid==0 -> nothing to report
    // pid <0 -> some system error ... ignore ?
    // pide>0 -> some pid has died (in one way or the other)
    if (pid > 0) {
      if (pid == m_pid_merger) {
        if (getNode().getState() == RCState::LOADING_TS
            || getNode().getState() == RCState::STARTING_TS || getNode().getState() == RCState::STOPPING_TS
            || getNode().getState() == RCState::READY_S || getNode().getState() == RCState::RUNNING_S) {
          // surpress Fatal and Error if we are in Abort? race condition on m_pid_merger
          m_log->Fatal("RoiSenderCallback : merger2merge dead. pid = %d\n", pid);
          setState(RCState::ERROR_ES);
        } else {
          m_log->Info("RoiSenderCallback : merger2merge dead. pid = %d\n", pid);
        }
        m_pid_merger = 0; // race condition for recover!!!
        // if you have many childs, do a continue here to avoid having to await timout of CheckOutput (1s)
      }
    } else if (pid < 0) {
      perror("RoiSenderCallback::server");
    }
    int st = m_proc->CheckOutput();
    // st==0 -> timeout of select
    // st <0 -> system error ... ?
    // st >0 some file descriptor has action
    if (st < 0) {
      perror("RoiSenderCallback::server");// we will never see that in the logging
    } else if (st > 0) {
      m_log->ProcessLog(m_proc->GetFd());
    }
  }
}
