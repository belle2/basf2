/// @file ring_bugger.cc
/// @brief Ring_bugger class implementation
/// @author Ryosuke Itoh
/// @date Feb. 22, 2010

//+
// File : ring_buffer.cc
// Description : Ring Buffer manager on shared memory
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 29 - Apr - 2000
//-

#include <framework/pcore/RingBuffer.h>
#include <framework/logging/Logger.h>

#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <boost/format.hpp>

#define REDZONE_FACTOR 0.8

using namespace std;
using namespace Belle2;

// Constructor / Destructor

RingBuffer::RingBuffer(char* name, int size)
{
  // 1. Open shared memory
  m_shmid = shmget(IPC_PRIVATE, size * 4, IPC_CREAT | 0644);
  if (m_shmid < 0) {
    perror("RingBuffer::shmget");
    return;
  }
  m_shmadr = (int *) shmat(m_shmid, 0, 0);
  if (m_shmadr == (int*) - 1) {
    perror("RingBuffer::shmat");
    return;
  }
  //  cout << "Shared Memory created" << endl;

  // 2. Open Semaphore
  m_semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0644);
  if (m_semid < 0) {
    perror("RingBuffer::semget");
    return;
  }
  //  cout << "Semaphore created" << endl;

  // 3. Initialize control parameters
  m_shmsize = size;
  m_bufinfo = (struct RingBufInfo*) m_shmadr;
  m_buftop = m_shmadr + sizeof(struct RingBufInfo);
  m_bufinfo->size = m_shmsize - sizeof(struct RingBufInfo);
  m_bufinfo->remain = m_bufinfo->size;
  m_bufinfo->wptr = 0;
  m_bufinfo->prevwptr = 0;
  m_bufinfo->rptr = 0;
  m_bufinfo->nbuf = 0;
  m_bufinfo->semid = m_semid;
  m_bufinfo->nattached = 1;
  m_bufinfo->mode = 0;
  m_bufinfo->ninsq = 0;
  m_bufinfo->nremq = 0;

  m_remq_counter = 0;
  m_insq_counter = 0;

  sem_unlock(m_semid);
  cout << "RingBuffer initialization done" << endl;
  B2INFO(boost::format("buftop = %1%, end = %2%\n") % m_buftop % (m_buftop + m_bufinfo->size))
}

RingBuffer::RingBuffer(int shm_id)
{
  m_shmid = shm_id;
  m_shmadr = (int *) shmat(m_shmid, 0, 0);
  if (m_shmadr == (int*) - 1) {
    perror("RingBuffer::shmat");
    return;
  }
  m_bufinfo = (struct RingBufInfo*) m_shmadr;
  m_buftop = m_shmadr + sizeof(RingBufInfo);
  m_semid = m_bufinfo->semid;
  m_bufinfo->nattached++;

  m_remq_counter = 0;
  m_insq_counter = 0;

}

RingBuffer::~RingBuffer(void)
{
  cleanup();
}

void RingBuffer::cleanup(void)
{
  shmdt((char*)m_shmadr);
  printf("RingBuffer: Cleaning up IPC\n");
  shmctl(m_shmid, IPC_RMID, (struct shmid_ds*) 0);
  struct sembuf arg;
  semctl(m_semid, 1, IPC_RMID, arg);
}

void RingBuffer::dump_db(void)
{
  printf("bufsize=%d, remain=%d, wptr=%d, rptr=%d, nbuf=%d\n",
         m_bufinfo->size, m_bufinfo->remain,
         m_bufinfo->wptr, m_bufinfo->rptr, m_bufinfo->nbuf);
}

// Func

int RingBuffer::insq(int* buf, int size)
{
  //  printf ( "insq: requesting : %d, nbuf = %d\n", size, m_bufinfo->nbuf );
  if (size < 0) {
    printf("RingBuffer::insq : buffer size = %d, not queued.\n", size);
    return -1;
  }
  sem_lock(m_semid);
  if (m_bufinfo->nbuf == 0) {
    m_bufinfo->wptr = 0;
    m_bufinfo->rptr = 0;
    if (size > m_bufinfo->size + 2) {
      printf("insq: buffer too large!! %d\n", size);
      return -1;
    }
    m_bufinfo->mode = 0;
    int* wptr = m_buftop + m_bufinfo->wptr;
    *wptr = size;
    *(wptr + 1) = m_bufinfo->wptr + (size + 2);
    memcpy(wptr + 2, buf, size*4);
    m_bufinfo->prevwptr = m_bufinfo->wptr;
    m_bufinfo->wptr += (size + 2);
    m_bufinfo->nbuf++;
    //    printf ( "insq: nbuf = 0; prev=%d, new=%d\n",
    //       m_bufinfo->prevwptr, m_bufinfo->wptr );
    m_bufinfo->ninsq++;
    m_insq_counter++;
    sem_unlock(m_semid);
    return size;
  } else if (m_bufinfo->wptr > m_bufinfo->rptr) {
    if (m_bufinfo->mode != 4 &&
        m_bufinfo->mode != 3 && m_bufinfo->mode != 0) {
      printf("insq: Error in mode 0; current=%d\n", m_bufinfo->mode);
      sem_unlock(m_semid);
      return -1;
    }
    if (m_bufinfo->mode == 3) {
      //      printf ( "---> mode is 3, still remaining buffers\n" );
      return -1;
    } else if (m_bufinfo->mode == 4) {
      //      printf ( "---> mode returned to 0, wptr=%d, rptr=%d\n",
      //         m_bufinfo->wptr, m_bufinfo->rptr );
    }
    m_bufinfo->mode = 0;
    if (size + 2 < m_bufinfo->size - m_bufinfo->wptr) { // normal case
      int* wptr = m_buftop + m_bufinfo->wptr;
      *wptr = size;
      *(wptr + 1) = m_bufinfo->wptr + (size + 2);
      memcpy(wptr + 2, buf, size*4);
      m_bufinfo->prevwptr = m_bufinfo->wptr;
      m_bufinfo->wptr += (size + 2);
      m_bufinfo->nbuf++;
      //      printf ( "insq: wptr>rptr and enough size; prev=%d, new=%d\n",
      //         m_bufinfo->prevwptr, m_bufinfo->wptr );
      m_bufinfo->ninsq++;
      m_insq_counter++;
      sem_unlock(m_semid);
      return size;
    } else {
      if (m_bufinfo->rptr >= size + 2) { // buffer full and wptr>rptr
        if (m_bufinfo->mode != 0) {
          printf("insq: Error in mode 1; current=%d\n", m_bufinfo->mode);
          sem_unlock(m_semid);
          return -1;
        }
        m_bufinfo->mode = 1;
        int* wptr = m_buftop;
        memcpy(wptr + 2, buf, size*4);
        *wptr = size;
        *(wptr + 1) = size + 2;
        m_bufinfo->wptr = *(wptr + 1);
        int* prevptr = m_buftop + m_bufinfo->prevwptr;
        *(prevptr + 1) = 0;
        m_bufinfo->prevwptr = 0;
        if (m_bufinfo->nbuf == 0) {
          //    printf ( "===> rptr reset......\n" );
          m_bufinfo->mode = 4;
          m_bufinfo->rptr = 0;
        }
        m_bufinfo->nbuf++;
        //  printf ( "insq: no more space, space below rptr; prev=%d, new=%d\n",
        //         m_bufinfo->prevwptr, m_bufinfo->wptr );
        m_bufinfo->ninsq++;
        m_insq_counter++;
        sem_unlock(m_semid);
        return size;
      } else {
        //  printf ( "insq: wptr>rptr, no more space, no space below rptr(%d), readbuf=%d\n",
        //     m_bufinfo->rptr, m_bufinfo->readbuf );
        sem_unlock(m_semid);
        return -1;
      }
    }
  } else {  // wptr < rptr
    if (m_bufinfo->wptr + size + 2 < m_bufinfo->rptr &&
        size + 2 < m_bufinfo->size - m_bufinfo->rptr) {
      if (m_bufinfo->mode != 1 && m_bufinfo->mode != 2 &&
          m_bufinfo->mode != 3) {
        printf("insq: Error in mode 2; current=%d\n", m_bufinfo->mode);
        sem_unlock(m_semid);
        return (-1);
      }
      m_bufinfo->mode = 2;
      int* wptr = m_buftop + m_bufinfo->wptr;
      *wptr = size;
      *(wptr + 1) = m_bufinfo->wptr + (size + 2);
      memcpy(wptr + 2, buf, size*4);
      m_bufinfo->prevwptr = m_bufinfo->wptr;
      m_bufinfo->wptr += (size + 2);
      m_bufinfo->nbuf++;
      //      printf ( "insq: wptr<rptr and enough space below rptr; curr=%d, next=%d, rptr=%d\n", m_bufinfo->prevwptr, m_bufinfo->wptr, m_bufinfo->rptr );
      if (m_bufinfo->wptr > m_bufinfo->rptr) {
        printf("next pointer will exceed rptr.....\n");
        m_bufinfo->mode = 3;
      }
      m_bufinfo->ninsq++;
      m_insq_counter++;
      sem_unlock(m_semid);
      return size;
    } else {
      //      printf ( "insq: wptr<rptr; no more space below rptr(%d), wptr(%d)\n",
      //         m_bufinfo->rptr, m_bufinfo->wptr );
      sem_unlock(m_semid);
      return -1;
    }
  }
}

int RingBuffer::remq(int* buf)
{
  sem_lock(m_semid);
  if (m_bufinfo->nbuf <= 0) {
    sem_unlock(m_semid);
    return 0;
  }
  //  printf ( "remq : nbuf = %d\n", m_bufinfo->nbuf );
  int* r_ptr = m_buftop + m_bufinfo->rptr;
  int nw = *r_ptr;
  if (nw <= 0) {
    printf("RingBuffer::remq : buffer size = %d, skipped\n", nw);
    sem_unlock(m_semid);
    return 0;
  }
  //  printf ( "remq : taking buf from %d(%d)\n", m_bufinfo->rptr, nw );
  memcpy(buf, r_ptr + 2, nw*4);
  m_bufinfo->rptr = *(r_ptr + 1);
  //  if ( *(r_ptr+1) < m_bufinfo->rptr )
  if (m_bufinfo->rptr == 0)
    m_bufinfo->mode = 4;
  m_bufinfo->nbuf--;
  //  printf ( "remq: next buf = %d, nbuf = %d\n", m_bufinfo->rptr, m_bufinfo->nbuf );
  m_bufinfo->nremq++;
  m_remq_counter++;
  sem_unlock(m_semid);
  return nw;
}

int RingBuffer::numq(void)
{
  return m_bufinfo->nbuf;
}

int RingBuffer::ninsq(void)
{
  return m_bufinfo->ninsq;
}

int RingBuffer::nremq(void)
{
  return m_bufinfo->nremq;
}

int RingBuffer::insq_counter(void)
{
  return m_insq_counter;
}

int RingBuffer::remq_counter(void)
{
  return m_remq_counter;
}

int RingBuffer::clear(void)
{
  sem_lock(m_semid);
  //  m_bufinfo->size = m_shmsize - sizeof ( struct RingBufInfo );
  m_bufinfo->remain = m_bufinfo->size;
  m_bufinfo->wptr = 0;
  m_bufinfo->prevwptr = 0;
  m_bufinfo->rptr = 0;
  m_bufinfo->nbuf = 0;
  m_bufinfo->ninsq = 0;
  m_bufinfo->nremq = 0;
  sem_unlock(m_semid);

  return 0;
}

int RingBuffer::shmid(void)
{
  return m_shmid;
}

int RingBuffer::sem_lock(int sid)
{
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_op = -1;
  sb.sem_flg = 0;
  if (semop(sid, &sb, 1) == -1)
    fprintf(stderr, "Ringbuffer: error in sem_lock(semop) %d, %s\n",
            sid, strerror(errno));

  //  printf ( "semaphore locked.....\n" );
  return 0;
}

int RingBuffer::sem_unlock(int sid)
{
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_op = 1;
  sb.sem_flg = 0;
  if (semop(sid, &sb, 1) == -1)
    if (semop(sid, &sb, 1) == -1)
      fprintf(stderr, "Ringbuffer: error in sem_lock(semop) %d, %s\n",
              sid, strerror(errno));
  //    perror ("semop");
  //  printf ( "semaphore unlocked.....\n" );
  return 0;
}
