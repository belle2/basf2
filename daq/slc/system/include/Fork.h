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
    static void __handler_exit(int, void* worker) {
      delete(WORKER*)worker;
      exit(0);
    }
    static void __handler_int(int) { exit(0); }

  public:
    Fork() throw() : _pid(-1) {}

    template<class WORKER>
    Fork(WORKER* worker, bool detached = true) throw() {
      _pid = fork();
      if (_pid == 0) {
        signal(SIGINT, __handler_int);
        if (detached) {
          on_exit(__handler_exit<WORKER>, (void*)worker);
        }
        worker->run();
        exit(0);
      } else if (detached) {
        delete worker;
      }
    }
    ~Fork() throw() {}

  public:
    pid_t get_id() { return _pid; }
    bool kill(int signo) {
      if (_pid < 0) return false;
      return ::kill(_pid, signo) == 0;
    }
    bool wait(int opt = 0) {
      if (_pid < 0) return false;
      return ::waitpid(_pid, NULL, opt);
    }
    bool cancel() {
      //kill(SIGINT);
      //kill(SIGQUIT);
      return kill(SIGKILL);
    }

  private:
    pid_t _pid;

  };

}

#endif
