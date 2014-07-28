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
    B2FATAL("Couldn't create semaphore with semget()! Maybe you have semaphores from aborted processes lying around, you can clean those up using 'clear_basf2_ipc'. (Don't do this if you are currently running basf2 with parallel processing!)");
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
  if (semctl(semId, 1, IPC_RMID) == -1) { //semnum=1 has no meaning, ignored
    B2ERROR("Error in SemaphoreLocker::destroy(), semaphore " << semId << ", error: " << strerror(errno));
  }
}

bool SemaphoreLocker::isLocked(int semId)
{
  int ignored = 0;
  int val = semctl(semId, 0, GETVAL, ignored);
  B2WARNING("Semaphore value is : " << val);
  return (val == 0); //0: locked, 1: unlocked

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
      B2FATAL("Error in SemaphoreLocker::lock(), semaphore " << m_id << ", error: " << strerror(errno));
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
      B2FATAL("Error in SemaphoreLocker::unlock(), semaphore " << m_id << ", error: " << strerror(errno));
    }
  }
}

