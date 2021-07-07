/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/rfarm/manager/SharedMem.h"

#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

using namespace Belle2;
using namespace std;

// Constructor / Destructor

SharedMem::SharedMem(const char* name, int size)
{
  // 0. Determine shared memory type
  if (strcmp(name, "private") != 0) {      // Global
    m_file = true;
    m_pathname = string("/tmp/") + string(getenv("USER"))
                 + string("_SHM_") + string(name);
    m_pathfd = open(m_pathname.c_str(), O_CREAT | O_EXCL | O_RDWR, 0644);
    if (m_pathfd > 0) {   // a new shared memory file created
      printf("SharedMem: Creating a shared memory with key %s\n", name);
      m_new = true;
    } else if (m_pathfd == -1 && errno == EEXIST) { // shm already there
      printf("SharedMem: Attaching the ring buffer with key %s\n", name);
      m_new = false;
    } else {
      printf("SharedMem: error to open shm file\n");
      return;
    }
    m_shmkey = ftok(m_pathname.c_str(), 1);
    m_semkey = ftok(m_pathname.c_str(), 2);
  } else { // Private
    m_file = false;
    m_new = true;
    m_shmkey = IPC_PRIVATE;
    m_semkey = IPC_PRIVATE;
    printf("SharedMem: Opening private shared memory\n");
  }

  // 1. Open shared memory
  m_shmid = shmget(m_shmkey, size * 4, IPC_CREAT | 0644);
  if (m_shmid < 0) {
    perror("SharedMem::shmget");
    return;
  }
  m_shmadr = (int*) shmat(m_shmid, 0, 0);
  if (m_shmadr == (int*) - 1) {
    perror("SharedMem::shmat");
    return;
  }
  m_shmsize = size;

  // 2. Open semaphore
  m_semid = semget(m_semkey, 1, IPC_CREAT | 0644);
  if (m_semid >= 0) {
    // POSIX doesn't guarantee any particular state of our fresh semaphore
    int semval = 1; //unlocked state
    if (semctl(m_semid, 0, SETVAL, semval) == -1) { //set 0th semaphore to semval
      printf("Initializing semaphore with semctl() failed.\n");
    }
  } else if (errno == EEXIST) {
    m_semid = semget(m_semkey, 1, 0600);
  }
  if (m_semid < 0) {
    perror("SharedMem::shmget");
    return;
  }

  // 3. Leave id of shm and semaphore in file name
  if (m_new) {
    /*
    m_strbuf = new char[1024];
    sprintf(m_strbuf, "/tmp/SHM%d-SEM%d-SHM_%s", m_shmid, m_semid, name);
    int fd = open(m_strbuf, O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd < 0) {
      printf("SharedMem ID file could not be created.\n");
    } else {
      close(fd);
    }
    */
    //    printf("SharedMem: leaving shmid and semid in the path file %d %d fd=%d\n", m_shmid, m_semid, m_pathfd);
    char shminfo[256];
    sprintf(shminfo, "%d %d\n", m_shmid, m_semid);
    int is = write(m_pathfd, shminfo, strlen(shminfo));
    if (is < 0) perror("write");
    close(m_pathfd);
  }
  printf("SharedMem: created. shmid = %d, semid = %d\n", m_shmid, m_semid);

}

SharedMem::SharedMem(int shm_id)
{
  m_shmid = shm_id;
  m_shmadr = (int*) shmat(m_shmid, 0, 0);
  if (m_shmadr == (int*) - 1) {
    perror("SharedMem::shmat");
    return;
  }
}

SharedMem::SharedMem(int shm_id, int sem_id, int size)
{
  m_shmid = shm_id;
  m_shmadr = (int*) shmat(m_shmid, 0, SHM_RDONLY);
  if (m_shmadr == (int*) - 1) {
    perror("SharedMem::shmat");
    return;
  }
  m_shmsize = size;
  m_semid = sem_id;
  printf("SharedMem: open shmid = %d, semid = %d\n", m_shmid, m_semid);
}

SharedMem::~SharedMem(void)
{
  shmdt((const void*) m_shmadr);
  shmctl(m_shmid, IPC_RMID, NULL);
  //  char idfile[256];
  //  sprintf ( idfile, "%s/.rfshmid", getenv("HOME") );
  //  unlink ( idfile );
  if (m_new) {
    unlink(m_strbuf);
    delete[] m_strbuf;
  }
  printf("SharedMem: destructor called for %s\n", m_strbuf);
}

void* SharedMem::ptr(void)
{
  return (void*) m_shmadr;
}

int SharedMem::shmid(void)
{
  return m_shmid;
}

bool SharedMem::IsCreated(void)
{
  return m_new;
}

void SharedMem::lock()
{
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_op = -1;
  sb.sem_flg = 0;
  while (semop(m_semid, &sb, 1) == -1) {
    if (errno == EINTR) {
      //interrupted by signal (e.g. window size changed), try again
      continue;
    } else {
      perror("lock:semop");
      exit(-1);
    }
  }
}

void SharedMem::unlock()
{
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_op = 1;
  sb.sem_flg = 0;
  while (semop(m_semid, &sb, 1) == -1) {
    if (errno == EINTR) {
      //interrupted by signal (e.g. window size changed), try again
      continue;
    } else {
      perror("unlock:semop");
      exit(-1);
    }
  }
}

bool SharedMem::isLocked()
{
  int ignored = 0;
  int val = semctl(m_semid, 0, GETVAL, ignored);
  return (val == 0); //0: locked, 1: unlocked

}


