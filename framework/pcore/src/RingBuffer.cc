/// @file RingBuffer.cc
/// @brief RingBuffer class implementation
/// @author Ryosuke Itoh
/// @date Feb. 22, 2010

//+
// File : RingBuffer.cc
// Description : Ring Buffer manager on shared memory
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 29 - Apr - 2000
//-

#include <framework/pcore/RingBuffer.h>
#include <framework/logging/Logger.h>

#include <boost/format.hpp>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

using namespace std;
using namespace Belle2;


// Constructor of Private Ringbuffer
RingBuffer::RingBuffer(int size)
{
  m_file = false;
  m_new = true;
  m_pathname = "";
  m_shmkey = IPC_PRIVATE;


  // 1. Open shared memory
  m_shmid = shmget(IPC_PRIVATE, size * sizeof(int), IPC_CREAT | 0644);
  if (m_shmid < 0) {
    B2FATAL("RingBuffer: shmget(" << size * sizeof(int) << ") failed. Most likely the system doesn't allow us to reserve the needed shared memory. Try 'echo 500000000 > /proc/sys/kernel/shmmax' as root to set a higher limit (500MB).");
    return;
  }
  m_shmadr = (int*) shmat(m_shmid, 0, 0);
  if (m_shmadr == (int*) - 1) {
    B2FATAL("RingBuffer::shmat");
    return;
  }
  //  cout << "Shared Memory created" << endl;

  // 2. Open Semaphore
  m_semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0644);
  if (m_semid < 0) {
    B2FATAL("RingBuffer::semget");
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
  //  cout << "RingBuffer initialization done" << endl;
  B2INFO("RingBuffer initialization done");
  B2INFO(boost::format("buftop = %1%, end = %2%\n") % m_buftop % (m_buftop + m_bufinfo->size));
}

// Constructor of Global Ringbuffer with name
RingBuffer::RingBuffer(const char* name, unsigned int size)
{
  // 0. Determine shared memory type
  if (strcmp(name, "private") != 0) {      // Global
    m_file = true;
    m_pathname = string("/tmp/") + string(getenv("USER"))
                 + string("_") + string(name);
    //m_pathfd = creat ( m_pathname.c_str(), 0644 );
    m_pathfd = open(m_pathname.c_str(), O_CREAT | O_EXCL, 0644);
    //m_pathfd = open ( m_pathname.c_str(), O_EXCL, 0644 );
    if (m_pathfd > 0) {   // a new shared memory file created
      B2INFO("[RingBuffer] Creating a ring buffer with key " << name);
      m_new = true;
    } else if (m_pathfd == -1 && errno == EEXIST) { // shm already there
      B2INFO("[RingBuffer] Attaching the ring buffer with key " << name);
      m_new = false;
    } else {
      B2FATAL("RingBuffer: error to open shm file");
      return;
    }
    m_shmkey = ftok(m_pathname.c_str(), 1);
    m_semkey = ftok(m_pathname.c_str(), 2);
  } else { // Private
    m_file = false;
    m_new = true;
    m_shmkey = IPC_PRIVATE;
    m_semkey = IPC_PRIVATE;
    B2INFO("[RingBuffer] Opening private ring buffer");
  }

  // 1. Open shared memory
  m_shmid = shmget(m_shmkey, size * sizeof(int), IPC_CREAT | 0644);
  if (m_shmid < 0) {
    B2FATAL("RingBuffer: shmget(" << size * sizeof(int) << ") failed. Most likely the system doesn't allow us to reserve the needed shared memory. Try 'echo 500000000 > /proc/sys/kernel/shmmax' as root to set a higher limit (500MB).");
    return;
  }
  m_shmadr = (int*) shmat(m_shmid, 0, 0);
  if (m_shmadr == (int*) - 1) {
    B2FATAL("RingBuffer: shmat() failed");
    return;
  }

  // 2. Open Semaphore
  m_semid = semget(m_semkey, 1, IPC_CREAT | 0644);
  if (m_semid < 0) {
    B2FATAL("RingBuffer: semget() failed");
    return;
  }
  //  cout << "Semaphore created" << endl;

  // 3. Initialize control parameters
  m_shmsize = size;
  if (m_new) {
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
  } else {
    sem_lock(m_semid);    // Prevent simulnaneous initialization
    m_bufinfo = (struct RingBufInfo*) m_shmadr;
    m_buftop = m_shmadr + sizeof(struct RingBufInfo);
    m_bufinfo->nattached++;

    B2INFO("[RingBuffer] check entries = " << m_bufinfo->nbuf);
    B2INFO("[RingBuffer] check size = " << m_bufinfo->size);
  }

  m_remq_counter = 0;
  m_insq_counter = 0;

  sem_unlock(m_semid);

  //  cout << "RingBuffer initialization done" << endl;
  B2INFO("RingBuffer initialization done with shm=" << m_shmid);
  B2INFO(boost::format("buftop = %1%, end = %2%\n") % m_buftop % (m_buftop + m_bufinfo->size));
}

RingBuffer::~RingBuffer(void)
{
  cleanup();
}

void RingBuffer::cleanup(void)
{
  shmdt((char*)m_shmadr);
  B2INFO("RingBuffer: Cleaning up IPC");
  if (m_new) {
    shmctl(m_shmid, IPC_RMID, (struct shmid_ds*) 0);
    semctl(m_semid, 1, IPC_RMID);
    if (m_file) {
      close(m_pathfd);
      unlink(m_pathname.c_str());
    }
  }

}

void RingBuffer::dump_db(void)
{
  printf("bufsize=%d, remain=%d, wptr=%d, rptr=%d, nbuf=%d\n",
         m_bufinfo->size, m_bufinfo->remain,
         m_bufinfo->wptr, m_bufinfo->rptr, m_bufinfo->nbuf);
}

int RingBuffer::insq(const int* buf, int size)
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
      B2INFO("[RingBuffer::insq ()] Inserted item is too large! ("
             << m_bufinfo->size + 2 << " < " << size << ")");
      sem_unlock(m_semid);
      return -1;
    }
    m_bufinfo->mode = 0;
    int* wptr = m_buftop + m_bufinfo->wptr;
    *wptr = size;
    *(wptr + 1) = m_bufinfo->wptr + (size + 2);
    memcpy(wptr + 2, buf, size * sizeof(int));
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
      B2INFO("[RingBuffer] mode 3");
      sem_unlock(m_semid);
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
      memcpy(wptr + 2, buf, size * sizeof(int));
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
        memcpy(wptr + 2, buf, size * sizeof(int));
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
      memcpy(wptr + 2, buf, size * sizeof(int));
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
  //  printf ( "RingBuffer : nw = %d\n", nw );
  if (nw <= 0) {
    printf("RingBuffer::remq : buffer size = %d, skipped\n", nw);
    printf("RingBuffer::remq : entries = %d\n", m_bufinfo->nbuf);
    sem_unlock(m_semid);
    return 0;
  }
  //  printf ( "remq : taking buf from %d(%d)\n", m_bufinfo->rptr, nw );
  memcpy(buf, r_ptr + 2, nw * sizeof(int));
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

int RingBuffer::spyq(int* buf)
{
  sem_lock(m_semid);
  if (m_bufinfo->nbuf <= 0) {
    sem_unlock(m_semid);
    return 0;
  }
  //  printf ( "remq : nbuf = %d\n", m_bufinfo->nbuf );
  int* r_ptr = m_buftop + m_bufinfo->rptr;
  int nw = *r_ptr;
  //  printf ( "RingBuffer : nw = %d\n", nw );
  if (nw <= 0) {
    printf("RingBuffer::spyq : buffer size = %d, skipped\n", nw);
    printf("RingBuffer::spyq : entries = %d\n", m_bufinfo->nbuf);
    sem_unlock(m_semid);
    return 0;
  }
  //  printf ( "remq : taking buf from %d(%d)\n", m_bufinfo->rptr, nw );
  // Copy buffer without modifying management parameters.
  memcpy(buf, r_ptr + 2, nw * sizeof(int));
  // Exit
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
      fprintf(stderr, "Ringbuffer: error in sem_unlock(semop) %d, %s\n",
              sid, strerror(errno));
  //    perror ("semop");
  //  printf ( "semaphore unlocked.....\n" );
  return 0;
}
