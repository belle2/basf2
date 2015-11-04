#ifndef _Belle2_PThread_hh
#define _Belle2_PThread_hh

#include <daq/slc/system/LogFile.h>

#include <pthread.h>
#include <signal.h>
#include <cstdio>
#include <exception>
#include <iostream>

namespace Belle2 {

  class PThread {

  private:
    template<class WORKER>
    static void destroy(void* arg)
    {
      WORKER* worker = (WORKER*)arg;
      delete worker;
      worker = NULL;
    }
    template<class WORKER>
    static void* create_destroy(void* arg)
    {
      WORKER* worker = (WORKER*)arg;
      pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_cleanup_push(destroy<WORKER>, arg);
      try {
        worker->run();
      } catch (const std::exception& e) {
        LogFile::fatal(e.what());
      }
      pthread_cleanup_pop(1);
      return NULL;
    }
    template<class WORKER>
    static void* create(void* arg)
    {
      WORKER* worker = (WORKER*)arg;
      pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      try {
        worker->run();
      } catch (const std::exception& e) {
        LogFile::fatal(e.what());
      }
      return NULL;
    }

  public:
    static void exit() { pthread_exit(NULL); }

  public:
    PThread() throw() : m_th(0) {}

    template<class WORKER>
    PThread(WORKER* worker, bool destroy = true, bool detached = true) throw()
    {
      m_th = 0;
      if (destroy) {
        if (pthread_create(&m_th, NULL, PThread::create_destroy<WORKER>,
                           (void*)worker) != 0) {
          m_th = 0;
        }
      } else {
        if (pthread_create(&m_th, NULL, PThread::create<WORKER>,
                           (void*)worker) != 0) {
          m_th = 0;
        }
      }
      if (detached) detach();
    }
    ~PThread() throw() {}

  public:
    pthread_t id() { return m_th; }
    pthread_t get_id() { return m_th; }
    bool kill(int signo)
    {
      if (m_th == 0) return false;
      return ::pthread_kill(m_th, signo) == 0;
    }
    bool is_alive() { return this->kill(0); }
    bool detach()
    {
      if (m_th == 0) return false;
      return ::pthread_detach(m_th) == 0;
    }
    bool join()
    {
      if (m_th == 0) return false;
      return ::pthread_join(m_th, NULL) == 0;
    }
    bool cancel()
    {
      if (m_th == 0) return false;
      return ::pthread_cancel(m_th) == 0;
    }

  private:
    pthread_t m_th;

  };

}

#endif
