//+
// File : sharedmem.cc
// Description : Shared memory library for comminication among processes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 11 - Dec - 2003
//-

#include "daq/rfarm/manager/SharedMem.h"

using namespace Belle2;
using namespace std;

// Constructor / Destructor

SharedMem::SharedMem(char* name, int size)
{
  // 0. Determine shared memory type
  if (strcmp(name, "private") != 0) {      // Global
    m_file = true;
    m_pathname = string("/tmp/") + string(getenv("USER"))
                 + string("_SHM_") + string(name);
    m_pathfd = open(m_pathname.c_str(), O_CREAT | O_EXCL, 0644);
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
  } else { // Private
    m_file = false;
    m_new = true;
    m_shmkey = IPC_PRIVATE;
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

SharedMem::~SharedMem(void)
{
  shmdt((const void*) m_shmadr);
  shmctl(m_shmid, IPC_RMID, NULL);
  //  char idfile[256];
  //  sprintf ( idfile, "%s/.rfshmid", getenv("HOME") );
  //  unlink ( idfile );
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


