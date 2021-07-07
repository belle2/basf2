/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_LockGuard_hh
#define _Belle2_LockGuard_hh

#include <daq/slc/system/Mutex.h>
#include "MMutex.h"

namespace Belle2 {
  /**
   * Lock Guard for a Mutex instance.
   * If a new instance of this class is generated,
   * the mutex is locked. On destruction, it is
   * automatically unlocked.
   * This allows to write very simple functions with locks, e.g.
   *
   *    Mutex m;
   *
   *    LockGuard lock(m);
   *    try {
   *      throw std::exception();
   *      // or
   *      return some_value;
   *    } catch(...) {
   *      std::cout << "Some exception" << std::endl;
   *    }
   *
   * without the needs to think about unlocking it, as it is done automatically.
   * It toughly follows the C++11 lock_guard and RIIA (https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization)
   *
   * WARNING: the lock can not be unlocked on uncaught exceptions (because they terminate the program by definition).
   * This is not a problem with the mutex, as it is released by the operation system on program termination.
   *
   * The GenericLockGuard is a templated version of the lock guard. Use the specialised
   * LockGuard of you want to lock a Mutex or the MLockGuard for an MMutex.
   */
  template<class AMutex>
  class GenericLockGuard {
  public:
    /// Construct a new LockGuard locking the mutex
    explicit GenericLockGuard(AMutex& mutex) : m_mutex(mutex)
    {
      m_mutex.lock();
    }

    /// Automatically release the lock on destruction
    ~GenericLockGuard()
    {
      m_mutex.unlock();
    }

  private:
    /// Do not allow to copy a lock guard
    GenericLockGuard(const GenericLockGuard&);

    /// Do not allow to copy a lock guard
    GenericLockGuard& operator=(const GenericLockGuard&);

    /// The mutex hold by this lock guard
    AMutex& m_mutex;
  };

  typedef GenericLockGuard<Mutex> LockGuard;
  typedef GenericLockGuard<MMutex> MLockGuard;
}

#endif
