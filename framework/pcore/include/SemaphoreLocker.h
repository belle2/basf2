/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <sys/types.h>

namespace Belle2 {
  /** Handles creation, locking and unlocking of System V semaphores.
   *
   * create() is used to create a new semaphore with a given semaphore key.
   *
   * The returned semaphore id can then be used to protect critical sections by
   * constructing an instance of this class. The semaphore is unlocked again on destruction.
   * */
  class SemaphoreLocker {
  public:
    /** Lock the given semaphore. */
    explicit SemaphoreLocker(int semId): m_id(semId) { lock(); }
    /** Unlock. */
    ~SemaphoreLocker() { unlock(); }

    /** Create a new semaphore and initialize it. Returns the semaphore id or value < 0 on error. */
    static int create(key_t semkey);
    /** Destroy the given semaphore. */
    static void destroy(int semId);

    /** Return true if the given semaphore is locked.
     *
     * If you know the lock might be held by the current process (e.g. while in a signal handler),
     * locking is only safe if this returns false.
     */
    static bool isLocked(int semId);

    /** Lock the semaphore.
     *
     * If the semaphore is locked, this function will block until it can acquire a lock.
     */
    void lock();
    /** Unlock the semaphore. */
    void unlock();

  private:
    int m_id; /**< semaphore id, see semget(2). */
  };
}
