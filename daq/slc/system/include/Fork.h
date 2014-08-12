#ifndef _Belle2_Fork_hh
#define _Belle2_Fork_hh

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

namespace Belle2 {

  class Fork {

  public:
    template <class WORKER>
    static void g_handler_exit(int, void* worker) {
      delete(WORKER*)worker;
      exit(0);
    }
    static void g_handler_int(int) { exit(0); }

  public:
    Fork() throw() : m_pid(-1) {}

    template<class WORKER>
    Fork(WORKER* worker, bool detached = true) throw() {
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
    ~Fork() throw() {}

  public:
    pid_t get_id() const { return m_pid; }
    bool isAlive() const { return m_pid > 0; }
    bool kill(int signo) const {
      if (m_pid < 0) return false;
      return ::kill(m_pid, signo) == 0;
    }
    bool wait(int opt = 0) {
      if (m_pid < 0) return false;
      return ::waitpid(m_pid, NULL, opt);
    }
    bool cancel() {
      return kill(SIGINT);
      //kill(SIGINT);
      //kill(SIGQUIT);
      //return kill(SIGKILL);
    }

  private:
    pid_t m_pid;

  };

}

#endif
