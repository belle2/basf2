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
  bool hasFile = false;
  std::string tmpPathName;
  // 0. Determine shared memory type
  if (strcmp(name, "private") != 0) {      // Global
    int tmpFilefd = -1;
    hasFile = true;
    tmpPathName = getTmpFileName(getenv("USER"), name);
    tmpFilefd = open(tmpPathName.c_str(), O_CREAT | O_EXCL | O_RDWR, 0644);
    // existance of file does not really imply shared mem is existing.
    // -> better we do not rely and update the content anyway!
    if (tmpFilefd > 0) {   // a new shared memory file created
      printf("SharedMem: Creating a new tmp file %s\n", name);
      m_new = true;
      close(tmpFilefd); // will open again later
    } else if (tmpFilefd == -1 && errno == EEXIST) { // shm already there
      printf("SharedMem: Found existing tmp file %s\n", name);
      m_new = false;
    } else {
      printf("SharedMem: error to open tmp file %s\n", tmpPathName.c_str());
      return;
    }
    m_shmkey = ftok(tmpPathName.c_str(), 1);
    m_semkey = ftok(tmpPathName.c_str(), 2);
  } else { // Private
    hasFile = false;
    m_new = true;
    m_shmkey = IPC_PRIVATE;
    m_semkey = IPC_PRIVATE;
    printf("SharedMem: Opening private shared memory\n");
  }

  printf("Shared memory/Semaphore Keys: $%X $%X\n", m_shmkey, m_semkey);
  // Behavior:
  // - IPC_CREATE will open existing or create new one
  // - IPC_CREATE|IPC_EXCL will create new one and fail is existing
  // - 0 will open existing one and fails if not existing

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

  // 2. Open semaphore

  // Behavior:
  // - IPC_CREATE will open existing or create new one
  // - IPC_CREATE|IPC_EXCL will create new one and fail is existing
  // - 0 will open existing one and fails if not existing
  m_semid = semget(m_semkey, 1, IPC_CREAT | IPC_EXCL | 0666);
  if (m_semid >= 0) {
    // POSIX doesn't guarantee any particular state of our fresh semaphore
    int semval = 1; //unlocked state
    printf("Semaphore ID %d created for key $%X\n", m_semid, m_semkey);
    if (semctl(m_semid, 0, SETVAL, semval) == -1) { //set 0th semaphore to semval
      perror("Initializing semaphore with semctl() failed.");
      return;
    }
  } else if (errno == EEXIST) {
    m_semid = semget(m_semkey, 1, 0); // obtain existing one
    printf("Found existing Semaphore ID %d for key $%X\n", m_semid, m_semkey);
  }
  if (m_semid < 0) {
    perror("SharedMem::shmget");
    return;
  }

  // 3. Put id of shm and semaphore in tmp file
  if (hasFile) {
    // private shm dont have a file, thus need to skip
    bool updateneeded = m_new;
    // 3.1 check if id of shm and semaphore in tmp file need update
    if (!m_new) {
      int shmid = 0, semid = 0;
      if (getIdFromTmpFileName(tmpPathName.c_str(), shmid, semid)) {
        updateneeded = (shmid != m_shmid || semid != m_semid);
        printf("tmp file %s content still uptodate\n", tmpPathName.c_str());
      } else {
        updateneeded = true; // could not open file or empty
      }
    }
    // 3.2 put id of shm and semaphore in tmp file
    if (updateneeded) {
      char shminfo[256];
      int tmpFilefd = open(tmpPathName.c_str(), O_RDWR, 0644);
      if (tmpFilefd < 0) {
        printf("SharedMem: error to reopen tmp file %s\n", tmpPathName.c_str());
        return;
      }
      snprintf(shminfo, sizeof(shminfo), "%d %d\n", m_shmid, m_semid);
      int is = write(tmpFilefd, shminfo, strlen(shminfo));
      if (is < 0) perror("write");
      close(tmpFilefd);
      printf("tmp file %s has been updated with shminfo \"%s\"\n", tmpPathName.c_str(), shminfo);
    }
  }
  printf("SharedMem: created. shmid = %d, semid = %d\n", m_shmid, m_semid);

}

SharedMem::SharedMem(int shm_id, int sem_id, int size)
{
  m_shmid = shm_id;
  m_shmadr = (int*) shmat(m_shmid, 0, SHM_RDONLY);
  if (m_shmadr == (int*) - 1) {
    perror("SharedMem::shmat");
    return;
  }
  m_semid = sem_id;
  printf("SharedMem: open shmid = %d, semid = %d\n", m_shmid, m_semid);
}

SharedMem::~SharedMem(void)
{
  /// this killed the shared mems if the b2hlt... utils are run, thus commented!

  //shmdt((const void*) m_shmadr);
  //shmctl(m_shmid, IPC_RMID, NULL);
  //printf("SharedMem: destructor called for ID %d\n", m_shmid);
  // TODO: problem, neither semaphore nor tmp file are deleted if they exist
  // TODO: there is no guarantee that the destructor is called (e.g. on exit(), crash)
  // printf("SharedMem: destructor called for %s\n", m_strbuf);
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

std::string SharedMem::getTmpFileName(std::string user, std::string name)
{
  return string("/tmp/") + user + string("_SHM_") + name;
}

bool SharedMem::getIdFromTmpFileName(std::string filename, int& shmid, int& semid)
{
  char shminfo[256];
  int fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0) {
    printf("SharedMem: error to reopen tmp file %s\n", filename.c_str());
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
  return (semctl(m_semid, 0, GETVAL, ignored) == 0); //0: locked, 1: unlocked
}


