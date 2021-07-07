/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_Process_h
#define _Belle2_Process_h

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

namespace Belle2 {

  class Process {

  public:
    template <class WORKER>
    static void g_handler_exit(int, void* worker)
    {
      delete(WORKER*)worker;
      exit(0);
    }
    static void g_handler_int(int) { exit(0); }

  public:
    Process() : m_pid(-1) {}

    template<class WORKER>
    Process(WORKER* worker, bool detached = true)
    {
      m_pid = fork();
      if (m_pid == 0) {
        signal(SIGINT, g_handler_int);
        if (detached) {
          on_exit(g_handler_exit<WORKER>, (void*)worker);
        }
        worker->run();
        exit(0);
      } else if (detached) {
        delete worker;
      }
    }
    ~Process() {}

  public:
    pid_t get_id() const { return m_pid; }
    pid_t id() const { return m_pid; }
    void set_id(pid_t id) { m_pid = id; }
    bool isAlive() const { return kill(0); }
    bool kill(int signo) const
    {
      if (m_pid <= 0) return false;
      return ::kill(m_pid, signo) == 0;
    }
    bool wait(int opt = 0)
    {
      if (m_pid < 0) return false;
      if (m_waitpid_result = ::waitpid(m_pid, &m_waitpid_status, opt)) {
        m_pid = -1;
      }
      return true;
    }
    bool cancel() { return kill(SIGINT); }
    int waitpid_result() const
    {
      return m_waitpid_result;
    };
    int waitpid_status() const
    {
      return m_waitpid_status;
    };

  private:
    pid_t m_pid;
    int m_waitpid_result;
    int m_waitpid_status;

  };

}

#endif
