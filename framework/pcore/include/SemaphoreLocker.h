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
    SemaphoreLocker(int semId): m_id(semId) { lock(); }
    /** Unlock. */
    ~SemaphoreLocker() { unlock(); }

    /** Create a new semaphore and initialize it. Returns the semaphore id or dies with an error. */
    static int create(key_t semkey);
    /** Destroy the given semaphore. */
    static void destroy(int semId);

    /** Lock the semaphore. */
    void lock();
    /** Unlock the semaphore. */
    void unlock();

  private:
    int m_id; /**< semaphore id, see semget(2). */
  };
}
