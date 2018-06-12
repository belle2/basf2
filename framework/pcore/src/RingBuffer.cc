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
#include <framework/pcore/SemaphoreLocker.h>
#include <framework/logging/Logger.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <fstream>
#include <stdexcept>

using namespace std;
using namespace Belle2;

// Constructor of Private Ringbuffer
RingBuffer::RingBuffer(int size)
{
  m_file = false;
  m_new = true;
  m_procIsBusy = false;
  m_pathname = "";
  m_shmkey = IPC_PRIVATE;
  m_semkey = IPC_PRIVATE;

  openSHM(size);

  B2DEBUG(32, "RingBuffer initialization done");
}

// Constructor of Global Ringbuffer with name
RingBuffer::RingBuffer(const std::string& name, unsigned int nwords)
{
  m_procIsBusy = false;
  // 0. Determine shared memory type
  if (name != "private") { // Global
    m_file = true;
    m_pathname = string("/tmp/") + getenv("USER") + "_RB_" + name;
    m_pathfd = open(m_pathname.c_str(), O_CREAT | O_EXCL | O_RDWR, 0644);
    if (m_pathfd > 0) {   // a new shared memory file created
      B2DEBUG(32, "[RingBuffer] Creating a ring buffer with key " << name);
      m_new = true;
    } else if (m_pathfd == -1 && errno == EEXIST) { // shm already there
      B2DEBUG(32, "[RingBuffer] Attaching the ring buffer with key " << name);
      m_new = false;
    } else {
      B2FATAL("RingBuffer: error opening shm file: " << m_pathname);
    }
    m_shmkey = ftok(m_pathname.c_str(), 1);
    m_semkey = ftok(m_pathname.c_str(), 2);
  } else { // Private
    m_file = false;
    m_new = true;
    m_shmkey = IPC_PRIVATE;
    m_semkey = IPC_PRIVATE;
    B2DEBUG(32, "[RingBuffer] Opening private ring buffer");
  }

  openSHM(nwords);

  if (m_pathfd > 0) {
    B2DEBUG(32, "First global RingBuffer creation: writing SHM info to file.");
    char rbufinfo[256];
    snprintf(rbufinfo, sizeof(rbufinfo), "%d\n", m_shmid);
    int is = write(m_pathfd, rbufinfo, strlen(rbufinfo));
    if (is < 0) perror("write");
    snprintf(rbufinfo, sizeof(rbufinfo), "%d\n", m_semid);
    is = write(m_pathfd, rbufinfo, strlen(rbufinfo));
    if (is < 0) perror("write");
    close(m_pathfd);
  }


  B2DEBUG(32, "RingBuffer initialization done with shm=" << m_shmid);
}

RingBuffer::~RingBuffer()
{
  cleanup();
}

void RingBuffer::openSHM(int nwords)
{
  // 1. Open shared memory
  unsigned int sizeBytes = nwords * sizeof(int);
  const auto mode = IPC_CREAT | 0644;
  m_shmid = shmget(m_shmkey, sizeBytes, mode);
  if (m_shmid < 0) {
    unsigned int maxSizeBytes = sizeBytes;
    ifstream shmax("/proc/sys/kernel/shmmax");
    if (shmax.good())
      shmax >> maxSizeBytes;
    shmax.close();

    B2WARNING("RingBuffer: shmget(" << sizeBytes << ") failed, limiting to system maximum: " << maxSizeBytes);
    sizeBytes = maxSizeBytes;
    nwords = maxSizeBytes / sizeof(int);
    m_shmid = shmget(m_shmkey, sizeBytes, mode);
    if (m_shmid < 0) {
      B2FATAL("RingBuffer: shmget(" << sizeBytes <<
              ") failed. Most likely the system doesn't allow us to reserve the needed shared memory. Try 'echo 500000000 > /proc/sys/kernel/shmmax' as root to set a higher limit (500MB).");
    }
  }
  m_shmadr = (int*) shmat(m_shmid, 0, 0);
  if (m_shmadr == (int*) - 1) {
    B2FATAL("RingBuffer: Attaching to shared memory segment via shmat() failed");
  }

  // 2. Open Semaphore
  m_semid = SemaphoreLocker::create(m_semkey);
  if (m_semid < 0) {
    cleanup();
    B2FATAL("Aborting execution because we couldn't create a semaphore (see previous error messages for details).");
  }
  SemaphoreLocker locker(m_semid); //prevent simultaneous initialization

  // 3. Initialize control parameters
  m_shmsize = nwords;
  m_bufinfo = reinterpret_cast<RingBufInfo*>(m_shmadr);
  m_buftop = m_shmadr + sizeof(struct RingBufInfo);
  if (m_new) {
    m_bufinfo->size = m_shmsize - sizeof(struct RingBufInfo);
    m_bufinfo->remain = m_bufinfo->size;
    m_bufinfo->wptr = 0;
    m_bufinfo->prevwptr = 0;
    m_bufinfo->rptr = 0;
    m_bufinfo->nbuf = 0;
    m_bufinfo->semid = m_semid;
    m_bufinfo->nattached = 1;
    m_bufinfo->nbusy = 0;
    m_bufinfo->errtype = 0;
    m_bufinfo->numAttachedTx = -1;
    m_bufinfo->ninsq = 0;
    m_bufinfo->nremq = 0;
  } else {
    m_bufinfo->nattached++;

    B2DEBUG(32, "[RingBuffer] check entries = " << m_bufinfo->nbuf);
    B2DEBUG(32, "[RingBuffer] check size = " << m_bufinfo->size);
  }

  m_remq_counter = 0;
  m_insq_counter = 0;

  if (m_new) {
    // Leave id of shm and semaphore in file name
    m_semshmFileName = "/tmp/SHM" + to_string(m_shmid) + "-SEM" + to_string(m_semid) + "-UNNAMED";
    int fd = open(m_semshmFileName.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd < 0) {
      B2WARNING("RingBuffer ID file could not be created.");
    } else {
      close(fd);
    }
  }

  B2DEBUG(35, "buftop = " << m_buftop << ", end = " << (m_buftop + m_bufinfo->size));
}

void RingBuffer::cleanup()
{
  if (m_procIsBusy) {
    SemaphoreLocker locker(m_semid);
    m_bufinfo->nbusy--;
    m_procIsBusy = false;
  }

  shmdt(m_shmadr);
  B2DEBUG(32, "RingBuffer: Cleaning up IPC");
  if (m_new) {
    shmctl(m_shmid, IPC_RMID, (struct shmid_ds*) 0);
    SemaphoreLocker::destroy(m_semid);
    if (m_file) {
      unlink(m_pathname.c_str());
    }
    unlink(m_semshmFileName.c_str());
  }
}

void RingBuffer::dump_db()
{
  printf("bufsize=%d, remain=%d, wptr=%d, rptr=%d, nbuf=%d\n",
         m_bufinfo->size, m_bufinfo->remain,
         m_bufinfo->wptr, m_bufinfo->rptr, m_bufinfo->nbuf);
}

int RingBuffer::insq(const int* buf, int size)
{
  if (size <= 0) {
    B2FATAL("RingBuffer::insq() failed: invalid buffer size = " << size);
  }
  if (m_bufinfo->numAttachedTx == 0) {
    //safe abort was requested
    exit(0);
  }
  SemaphoreLocker locker(m_semid);
  if (m_bufinfo->nbuf == 0) {
    m_bufinfo->wptr = 0;
    m_bufinfo->rptr = 0;
    if (size > m_bufinfo->size + 2) {
      throw std::runtime_error("[RingBuffer::insq ()] Inserted item (size: " + std::to_string(size) +
                               ") is larger than RingBuffer (size: " + std::to_string(m_bufinfo->size + 2) + ")!");
      return -1;
    }
    m_bufinfo->errtype = 0;
    int* wptr = m_buftop + m_bufinfo->wptr;
    *wptr = size;
    *(wptr + 1) = m_bufinfo->wptr + (size + 2);
    memcpy(wptr + 2, buf, size * sizeof(int));
    m_bufinfo->prevwptr = m_bufinfo->wptr;
    m_bufinfo->wptr += (size + 2);
    m_bufinfo->nbuf++;
    m_bufinfo->ninsq++;
    m_insq_counter++;
    return size;
  } else if (m_bufinfo->wptr > m_bufinfo->rptr) {
    if (m_bufinfo->errtype != 4 &&
        m_bufinfo->errtype != 3 && m_bufinfo->errtype != 0) {
      B2ERROR("insq: Error in errtype 0; current=" << m_bufinfo->errtype);
      return -1;
    }
    if (m_bufinfo->errtype == 3) {
      //      printf ( "---> errtype is 3, still remaining buffers\n" );
      B2DEBUG(32, "[RingBuffer] errtype 3");
      return -1;
    } else if (m_bufinfo->errtype == 4) {
      //      printf ( "---> errtype returned to 0, wptr=%d, rptr=%d\n",
      //         m_bufinfo->wptr, m_bufinfo->rptr );
    }
    m_bufinfo->errtype = 0;
    if (size + 2 < m_bufinfo->size - m_bufinfo->wptr) { // normal case
      int* wptr = m_buftop + m_bufinfo->wptr;
      *wptr = size;
      *(wptr + 1) = m_bufinfo->wptr + (size + 2);
      memcpy(wptr + 2, buf, size * sizeof(int));
      m_bufinfo->prevwptr = m_bufinfo->wptr;
      m_bufinfo->wptr += (size + 2);
      m_bufinfo->nbuf++;
      m_bufinfo->ninsq++;
      m_insq_counter++;
      return size;
    } else {
      if (m_bufinfo->rptr >= size + 2) { // buffer full and wptr>rptr
        if (m_bufinfo->errtype != 0) {
          B2ERROR("insq: Error in errtype 1; current=" << m_bufinfo->errtype);
          return -1;
        }
        m_bufinfo->errtype = 1;
        int* wptr = m_buftop;
        memcpy(wptr + 2, buf, size * sizeof(int));
        *wptr = size;
        *(wptr + 1) = size + 2;
        m_bufinfo->wptr = *(wptr + 1);
        int* prevptr = m_buftop + m_bufinfo->prevwptr;
        *(prevptr + 1) = 0;
        m_bufinfo->prevwptr = 0;
        if (m_bufinfo->nbuf == 0) {
          m_bufinfo->errtype = 4;
          m_bufinfo->rptr = 0;
        }
        m_bufinfo->nbuf++;
        //  printf ( "insq: no more space, space below rptr; prev=%d, new=%d\n",
        //         m_bufinfo->prevwptr, m_bufinfo->wptr );
        m_bufinfo->ninsq++;
        m_insq_counter++;
        return size;
      } else {
        //  printf ( "insq: wptr>rptr, no more space, no space below rptr(%d), readbuf=%d\n",
        //     m_bufinfo->rptr, m_bufinfo->readbuf );
        return -1;
      }
    }
  } else {  // wptr < rptr
    if (m_bufinfo->wptr + size + 2 < m_bufinfo->rptr &&
        size + 2 < m_bufinfo->size - m_bufinfo->rptr) {
      if (m_bufinfo->errtype != 1 && m_bufinfo->errtype != 2 &&
          m_bufinfo->errtype != 3) {
        B2ERROR("insq: Error in errtype 2; current=" << m_bufinfo->errtype);
        return -1;
      }
      m_bufinfo->errtype = 2;
      int* wptr = m_buftop + m_bufinfo->wptr;
      *wptr = size;
      *(wptr + 1) = m_bufinfo->wptr + (size + 2);
      memcpy(wptr + 2, buf, size * sizeof(int));
      m_bufinfo->prevwptr = m_bufinfo->wptr;
      m_bufinfo->wptr += (size + 2);
      m_bufinfo->nbuf++;
      //      printf ( "insq: wptr<rptr and enough space below rptr; curr=%d, next=%d, rptr=%d\n", m_bufinfo->prevwptr, m_bufinfo->wptr, m_bufinfo->rptr );
      if (m_bufinfo->wptr > m_bufinfo->rptr) {
        B2DEBUG(32, "next pointer will exceed rptr.....");
        m_bufinfo->errtype = 3;
      }
      m_bufinfo->ninsq++;
      m_insq_counter++;
      return size;
    } else {
      //      printf ( "insq: wptr<rptr; no more space below rptr(%d), wptr(%d)\n",
      //         m_bufinfo->rptr, m_bufinfo->wptr );
      return -1;
    }
  }
}

int RingBuffer::remq(int* buf)
{
  SemaphoreLocker locker(m_semid);
  if (m_bufinfo->nbuf < 0) {
    throw std::runtime_error("[RingBuffer::remq ()] number of entries is negative: " + std::to_string(m_bufinfo->nbuf));
  }
  if (m_bufinfo->nbuf == 0) {
    if (m_procIsBusy) {
      m_bufinfo->nbusy--;
      m_procIsBusy = false;
    }
    return 0;
  }
  int* r_ptr = m_buftop + m_bufinfo->rptr;
  int nw = *r_ptr;
  if (nw <= 0) {
    printf("RingBuffer::remq : buffer size = %d, skipped\n", nw);
    printf("RingBuffer::remq : entries = %d\n", m_bufinfo->nbuf);
    if (m_procIsBusy) {
      m_bufinfo->nbusy--;
      m_procIsBusy = false;
    }
    return 0;
  }
  if (buf)
    memcpy(buf, r_ptr + 2, nw * sizeof(int));
  m_bufinfo->rptr = *(r_ptr + 1);
  if (m_bufinfo->rptr == 0)
    m_bufinfo->errtype = 4;
  m_bufinfo->nbuf--;
  m_bufinfo->nremq++;
  m_remq_counter++;

  if (not m_procIsBusy) {
    m_bufinfo->nbusy++;
    m_procIsBusy = true;
  }
  return nw;
}

int RingBuffer::spyq(int* buf) const
{
  SemaphoreLocker locker(m_semid);
  if (m_bufinfo->nbuf <= 0) {
    return 0;
  }
  int* r_ptr = m_buftop + m_bufinfo->rptr;
  int nw = *r_ptr;
  if (nw <= 0) {
    printf("RingBuffer::spyq : buffer size = %d, skipped\n", nw);
    printf("RingBuffer::spyq : entries = %d\n", m_bufinfo->nbuf);
    return 0;
  }
  //  printf ( "remq : taking buf from %d(%d)\n", m_bufinfo->rptr, nw );
  // Copy buffer without modifying management parameters.
  memcpy(buf, r_ptr + 2, nw * sizeof(int));
  // Exit
  return nw;
}

int RingBuffer::numq() const
{
  return m_bufinfo->nbuf;
}

void RingBuffer::txAttached()
{
  SemaphoreLocker locker(m_semid);
  if (m_bufinfo->numAttachedTx == -1) //first attach
    m_bufinfo->numAttachedTx = 0;

  m_bufinfo->numAttachedTx++;
}
void RingBuffer::txDetached()
{
  SemaphoreLocker locker(m_semid);
  m_bufinfo->numAttachedTx--;
  if (m_bufinfo->numAttachedTx < 0) {
    m_bufinfo->numAttachedTx = 0;
  }
}
void RingBuffer::kill()
{
  m_bufinfo->numAttachedTx = 0;
  m_bufinfo->nbuf = 0;
}
bool RingBuffer::isDead() const
{
  SemaphoreLocker locker(m_semid);
  //NOTE: numAttachedTx == -1 also means we should read data (i.e. initialization pending)
  return (m_bufinfo->numAttachedTx == 0) and (m_bufinfo->nbuf <= 0);
}
bool RingBuffer::allRxWaiting() const
{
  SemaphoreLocker locker(m_semid);
  return (m_bufinfo->nbusy == 0) and (m_bufinfo->nbuf == 0);
}

int RingBuffer::ninsq() const
{
  return m_bufinfo->ninsq;
}

int RingBuffer::nremq() const
{
  return m_bufinfo->nremq;
}

int RingBuffer::insq_counter() const
{
  return m_insq_counter;
}

int RingBuffer::remq_counter() const
{
  return m_remq_counter;
}

int RingBuffer::clear()
{
  SemaphoreLocker locker(m_semid);
  //  m_bufinfo->size = m_shmsize - sizeof ( struct RingBufInfo );
  m_bufinfo->remain = m_bufinfo->size;
  m_bufinfo->wptr = 0;
  m_bufinfo->prevwptr = 0;
  m_bufinfo->rptr = 0;
  m_bufinfo->nbuf = 0;
  m_bufinfo->ninsq = 0;
  m_bufinfo->nremq = 0;

  return 0;
}

void RingBuffer::forceClear()
{
  int val = 1;
  if (semctl(m_semid, 0, SETVAL, val) == -1) { //set 0th semaphore to semval
    B2ERROR("Initializing semaphore with semctl() failed.");
  }
  clear();
}

int RingBuffer::tryClear()
{
  if (SemaphoreLocker::isLocked(m_semid))
    return 0;

  return clear();
}

int RingBuffer::shmid() const
{
  return m_shmid;
}

void RingBuffer::dumpInfo() const
{
  SemaphoreLocker locker(m_semid);

  // Dump control parameters
  printf("***** Ring Buffer Information ***\n");
  printf("path = %s\n", m_pathname.c_str());
  printf("shmsize = %d\n", m_shmsize);
  printf("[Buffer Info]\n");
  printf("bufsize = %d\n", m_bufinfo->size);
  printf("remain = %d\n", m_bufinfo->remain);
  printf("wptr = %d\n", m_bufinfo->wptr);
  printf("prevwptr = %d\n", m_bufinfo->prevwptr);
  printf("rptr = %d\n", m_bufinfo->rptr);
  printf("nbuf = %d\n", m_bufinfo->nbuf);
  printf("nattached = %d\n", m_bufinfo->nattached);
  printf("nbusy = %d\n", m_bufinfo->nbusy);
  printf("errtype = %d\n", m_bufinfo->errtype);
  printf("numAttachedTx = %d\n", m_bufinfo->numAttachedTx);
  printf("ninsq = %d\n", m_bufinfo->ninsq);
  printf("nremq = %d\n", m_bufinfo->ninsq);
}

