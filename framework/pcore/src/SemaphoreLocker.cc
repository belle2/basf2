#include <framework/pcore/SemaphoreLocker.h>
#include <framework/logging/Logger.h>

#include <sys/ipc.h>
#include <sys/sem.h>

#include <errno.h>
#include <string.h>

using namespace Belle2;

int SemaphoreLocker::create(key_t semkey)
{
  int semid = semget(semkey, 1, IPC_CREAT | 0600);
  if (semid < 0) {
    B2FATAL("Couldn't create semaphore with semget()!");
  }

  // POSIX doesn't guarantee any particular state of our fresh semaphore
  int semval = 1; //unlocked state
  if (semctl(semid, 0, SETVAL, semval) == -1) { //set 0th semaphore to semval
    B2FATAL("Initializing semaphore with semctl() failed.");
  }

  return semid;
}

void SemaphoreLocker::destroy(int semId)
{
  semctl(semId, 1, IPC_RMID); //semnum=1 has no meaning, ignored
}

void SemaphoreLocker::lock()
{
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_op = -1;
  sb.sem_flg = 0;
  while (semop(m_id, &sb, 1) == -1) {
    if (errno == EINTR) {
      //interrupted by signal (e.g. window size changed), try again
      continue;
    } else {
      B2FATAL("Ringbuffer: error in SemaphoreLocker::lock(semop), semaphore " << m_id << ", error: " << strerror(errno));
    }
  }
}

void SemaphoreLocker::unlock()
{
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_op = 1;
  sb.sem_flg = 0;
  while (semop(m_id, &sb, 1) == -1) {
    if (errno == EINTR) {
      //interrupted by signal (e.g. window size changed), try again
      continue;
    } else {
      B2FATAL("Ringbuffer: error in SemaphoreLocker::unlock(semop), semaphore " << m_id << ", error: " << strerror(errno));
    }
  }
}

