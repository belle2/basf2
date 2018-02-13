#include <eutel/eudaq/Mutex.h>
#include <eutel/eudaq/Exception.h>
#include <pthread.h>

namespace eudaq {

  class Mutex::Impl {
  public:
    Impl()
    {
      if (pthread_mutexattr_init(&m_attr)) {
        EUDAQ_THROW("Unable to create mutex attributes");
      }
      pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE);
      if (pthread_mutex_init(&m_mutex, &m_attr)) {
        EUDAQ_THROW("Unable to create mutex");
      }
    }
    ~Impl()
    {
      if (pthread_mutex_destroy(&m_mutex)) {
        // error
      }
    }
    pthread_mutexattr_t m_attr;
    pthread_mutex_t m_mutex;
  };

  Mutex::Mutex() : m_impl(new Mutex::Impl) {}

  Mutex::~Mutex() { delete m_impl; }

  int Mutex::Lock()
  {
    return pthread_mutex_lock(&m_impl->m_mutex);
  }

  int Mutex::TryLock()
  {
    return pthread_mutex_trylock(&m_impl->m_mutex);
  }

  int Mutex::UnLock()
  {
    return pthread_mutex_unlock(&m_impl->m_mutex);
  }

  MutexLock::MutexLock(Mutex& m, bool lock) : m_mutex(m), m_locked(lock)
  {
    if (lock && m_mutex.TryLock()) {
      EUDAQ_THROW("Unable to lock mutex");
    }
  }

  void MutexLock::Lock()
  {
    if (m_mutex.TryLock()) {
      EUDAQ_THROW("Unable to lock mutex");
    }
    m_locked = true;
  }

  void MutexLock::Release()
  {
    if (m_locked) m_mutex.UnLock();
    m_locked = false;
  }

  MutexLock::~MutexLock()
  {
    Release();
  }

//   MutexTryLock::MutexTryLock(Mutex & m) : m_mutex(m), m_locked(true) {
//     if (m_mutex.TryLock()) {
//       m_locked = false;
//     }
//   }

//   void MutexTryLock::Release() {
//     m_mutex.UnLock();
//     m_locked = false;
//   }

//   MutexTryLock::~MutexTryLock() {
//     if (m_locked) Release();
//   }

//   MutexTryLock::operator bool () const {
//     return m_locked;
//   }

}
