/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef SHAREDMEM_H
#define SHAREDMEM_H

#include <sys/types.h>

#include <string>

namespace Belle2 {
  class SharedMem {
  public:
    SharedMem(const char* name, int size);
    SharedMem(int shm_id);
    SharedMem(int shm_id, int sem_id, int size);
    ~SharedMem(void);

    void* ptr(void);
    int shmid(void);

    bool IsCreated(void);

    void lock();
    void unlock();
    bool isLocked();

  private:
    bool m_new; /**< True if we created the ring buffer ourselves (and need to clean it). */
    bool m_file;
    std::string m_pathname;
    int  m_pathfd; /** Associated file descriptor. */
    key_t m_shmkey; /**< SHM key, see shmget(2). */
    key_t m_semkey; /**< Semaphore key */

    int m_shmid;
    int m_semid;
    void* m_shmadr;
    int m_shmsize;
    char* m_strbuf;
  };
}

#endif

