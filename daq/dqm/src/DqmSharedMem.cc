/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/dqm/DqmSharedMem.h"

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

DqmSharedMem::DqmSharedMem(const char* name, int size)
{
  std::string tmpPathName;
  // 0. Determine shared memory type
  if (strcmp(name, "private") != 0) {      // Global
    tmpPathName = getTmpFileName(getenv("USER"), name);
    m_pathfd = open(tmpPathName.c_str(), O_CREAT | O_EXCL | O_RDWR, 0644);
    if (m_pathfd > 0) {   // a new shared memory file created
      printf("DqmSharedMem: Creating a shared memory with key %s\n", name);
      m_new = true;
    } else if (m_pathfd == -1 && errno == EEXIST) { // shm already there
      printf("DqmSharedMem: Attaching the ring buffer with key %s\n", name);
      m_new = false;
    } else {
      printf("DqmSharedMem: error to open shm file\n");
      return;
    }
    m_shmkey = ftok(tmpPathName.c_str(), 1);
    m_semkey = ftok(tmpPathName.c_str(), 2);
  } else { // Private
    m_new = true;
    m_shmkey = IPC_PRIVATE;
    m_semkey = IPC_PRIVATE;
    printf("DqmSharedMem: Opening private shared memory\n");
  }

  printf("Shared memory/Semaphore Keys: $%X $%X\n", m_shmkey, m_semkey);
  // Behavior:
  // - IPC_CREATE will open existing or create new one
  // - IPC_CREATE|IPC_EXCL will create new one and fail is existing
  // - 0 will open existing one and fails if not existing

  // 1. Open shared memory
  m_shmid = shmget(m_shmkey, size * 4, IPC_CREAT | 0644);
  if (m_shmid < 0) {
    perror("DqmSharedMem::shmget");
    return;
  }
  m_shmadr = (int*) shmat(m_shmid, 0, 0);
  if (m_shmadr == (int*) - 1) {
    perror("DqmSharedMem::shmat");
    return;
  }

  // 2. Open semaphore

  // Behavior:
  // - IPC_CREATE will open existing or create new one
  // - IPC_CREATE|IPC_EXCL will create new one and fail is existing
  // - 0 will open existing one and fails if not existing
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
    perror("DqmSharedMem::shmget");
    return;
  }

  // 3. Leave id of shm and semaphore in file name
  if (m_new) {
    /*
    m_strbuf = new char[1024];
    sprintf(m_strbuf, "/tmp/SHM%d-SEM%d-SHM_%s", m_shmid, m_semid, name);
    int fd = open(m_strbuf, O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd < 0) {
      printf("DqmSharedMem ID file could not be created.\n");
    } else {
      close(fd);
    }
    */
    //    printf("DqmSharedMem: leaving shmid and semid in the path file %d %d fd=%d\n", m_shmid, m_semid, m_pathfd);
    char shminfo[256];
    sprintf(shminfo, "%d %d\n", m_shmid, m_semid);
    int is = write(m_pathfd, shminfo, strlen(shminfo));
    if (is < 0) perror("write");
    close(m_pathfd);
  }
  printf("DqmSharedMem: created. shmid = %d, semid = %d\n", m_shmid, m_semid);

}

DqmSharedMem::DqmSharedMem(int shm_id, int sem_id, int size)
{
  m_shmid = shm_id;
  m_shmadr = (int*) shmat(m_shmid, 0, SHM_RDONLY);
  if (m_shmadr == (int*) - 1) {
    perror("DqmSharedMem::shmat");
    return;
  }
  m_semid = sem_id;
  printf("DqmSharedMem: open shmid = %d, semid = %d\n", m_shmid, m_semid);
}

DqmSharedMem::~DqmSharedMem(void)
{
  /// this killed the shared mems if the b2hlt... utils are run, thus commented!

  //shmdt((const void*) m_shmadr);
  //shmctl(m_shmid, IPC_RMID, NULL);
  //printf("DqmSharedMem: destructor called for ID %d\n", m_shmid);
  // TODO: problem, neither semaphore nor tmp file are deleted if they exist
  // TODO: there is no guarantee that the destructor is called (e.g. on exit(), crash)
  // printf("DqmSharedMem: destructor called for %s\n", m_strbuf);
}

void* DqmSharedMem::ptr(void)
{
  return (void*) m_shmadr;
}

int DqmSharedMem::shmid(void)
{
  return m_shmid;
}

bool DqmSharedMem::IsCreated(void)
{
  return m_new;
}

std::string DqmSharedMem::getTmpFileName(std::string user, std::string name)
{
  return string("/tmp/") + user + string("_SHM_") + name;
}

bool DqmSharedMem::getIdFromTmpFileName(std::string filename, int& shmid, int& semid)
{
  char shminfo[256];
  int fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0) {
    printf("DqmSharedMem: error to reopen tmp file %s\n", filename.c_str());
    return false;
  }
  shmid = -1;
  semid = -1;
  memset(shminfo, 0, sizeof(shminfo));
  int n = read(fd, shminfo, sizeof(shminfo));
  close(fd);
  sscanf(shminfo, "%d %d", &shmid, &semid);
  return (n >= 3 && shmid >= 0 && semid >= 0);
}

void DqmSharedMem::lock()
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

void DqmSharedMem::unlock()
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

bool DqmSharedMem::isLocked()
{
  int ignored = 0;
  return (semctl(m_semid, 0, GETVAL, ignored) == 0); //0: locked, 1: unlocked
}


