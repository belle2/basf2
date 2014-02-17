//+
// File : RbCtlMgr.cc
// Description : Manage control shared memory for roobasf
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Jan - 2010
//-

#include <framework/pcore/RbCtlMgr.h>

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

using namespace Belle2;

RbCtlMgr::RbCtlMgr()
{
  m_shmid = shmget(IPC_PRIVATE, sizeof(RbCtlShm), IPC_CREAT | 0600);
  if (m_shmid < 0) {
    perror("RbCtlMgr::shmget");
    return;
  }
  m_ctlshm = static_cast<RbCtlShm*>(shmat(m_shmid, 0, 0));
  m_ctlshm->n_sync = 0;

  m_semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
  if (m_semid < 0) {
    perror("RbCtlMgr::semget");
    return;
  }
  sem_unlock();
}

RbCtlMgr::RbCtlMgr(int shmid):
  m_shmid(shmid)
{
  m_ctlshm = static_cast<RbCtlShm*>(shmat(m_shmid, 0, 0));
}

RbCtlMgr::~RbCtlMgr()
{
  shmdt((const void*)m_ctlshm);
}


void RbCtlMgr::terminate()
{
  shmdt((const void*)m_ctlshm);
  shmctl(m_shmid, IPC_RMID, NULL);
  semctl(m_semid, 1, IPC_RMID);

}

RbCtlShm* RbCtlMgr::ctlshm()
{
  return m_ctlshm;
}

int RbCtlMgr::shmid()
{
  return m_shmid;
}

void RbCtlMgr::sync_set(int nprocess)
{
  sem_lock();
  m_ctlshm->n_sync = nprocess;
  sem_unlock();
  //  printf ( "RbCtlMgr: n_sync set to %d\n", m_ctlshm->n_sync );
}

void RbCtlMgr::sync_done()
{
  sem_lock();
  if (m_ctlshm->n_sync > 0) m_ctlshm->n_sync--;
  sem_unlock();
  //  printf ( "RbCtlMgr: n_sync decreased to %d\n", m_ctlshm->n_sync );
}

void RbCtlMgr::sync_wait()
{
  int count = 0;
  for (;;) {
    //    sem_lock();
    int nsync = m_ctlshm->n_sync;
    //    sem_unlock();
    if (nsync == 0) break;
    usleep(1000);
    count++;
    //    if ( count%1000 == 0 ) printf ( "RbCtlMgr: waiting : n_sync = %d\n",
    //            m_ctlshm->n_sync );
    if (count > 100000) {
      printf("RbCtlMgr: wait too long(100sec.) Just proceed\n");
      break;
    }
  }
}

void RbCtlMgr::set_flag(int id, int val)
{
  (m_ctlshm->flags)[id] = val;
}

int RbCtlMgr::get_flag(int id)
{
  return (m_ctlshm->flags)[id];
}


int RbCtlMgr::sem_lock()
{
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_op = -1;
  sb.sem_flg = 0;
  if (semop(m_semid, &sb, 1) == -1)
    fprintf(stderr, "RbCtlMgr: error in sem_lock(semop) %d, %s\n",
            m_semid, strerror(errno));

  //  printf ( "semaphore locked.....\n" );
  return 0;
}

int RbCtlMgr::sem_unlock()
{
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_op = 1;
  sb.sem_flg = 0;
  if (semop(m_semid, &sb, 1) == -1)
    fprintf(stderr, "RbCtlMgr: error in sem_lock(semop) %d, %s\n",
            m_semid, strerror(errno));
  //    perror ("semop");
  //  printf ( "semaphore unlocked.....\n" );
  return 0;
}



