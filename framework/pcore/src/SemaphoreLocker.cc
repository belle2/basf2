/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pcore/SemaphoreLocker.h>
#include <framework/logging/Logger.h>

#include <sys/ipc.h>
#include <sys/sem.h>

#include <cerrno>
#include <cstring>

using namespace Belle2;

int SemaphoreLocker::create(key_t semkey)
{
  int semid = semget(semkey, 1, IPC_CREAT | IPC_EXCL | 0600);
  if (semid >= 0) {
    // POSIX doesn't guarantee any particular state of our fresh semaphore
    int semval = 1; //unlocked state
    if (semctl(semid, 0, SETVAL, semval) == -1) { //set 0th semaphore to semval
      B2ERROR("Initializing semaphore with semctl() failed.");
    }
  } else if (errno == EEXIST) {
    semid = semget(semkey, 1, 0600);
  }
  if (semid < 0) {
    B2ERROR("Couldn't create semaphore with semget()! Maybe you have too many semaphores from aborted processes lying around, you can clean those up by running 'clear_basf2_ipc'.");
    return semid;
  }

  return semid;
}

void SemaphoreLocker::destroy(int semId)
{
  if (semctl(semId, 1, IPC_RMID) == -1) { //semnum=1 has no meaning, ignored
    B2ERROR("Error in SemaphoreLocker::destroy(), semaphore " << semId << ", error: " << strerror(errno));
  }
}

bool SemaphoreLocker::isLocked(int semId)
{
  int ignored = 0;
  int val = semctl(semId, 0, GETVAL, ignored);
  return (val == 0); //0: locked, 1: unlocked

}

namespace {
  void doSemOp(int semID, int op)
  {
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = op;
    sb.sem_flg = 0;
    while (semop(semID, &sb, 1) == -1) {
      if (errno == EINTR) {
        //interrupted by signal (e.g. window size changed), try again
        continue;
      } else {
        B2FATAL("Another process was aborted, please check previous output for reasons. (" <<
                strerror(errno) << " for semaphore " << semID << ")");
      }
    }
  }
}
void SemaphoreLocker::lock()
{
  doSemOp(m_id, -1);
}

void SemaphoreLocker::unlock()
{
  doSemOp(m_id, 1);
}

