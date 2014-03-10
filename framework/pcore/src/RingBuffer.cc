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

#include <fstream>

using namespace std;
using namespace Belle2;


// Constructor of Private Ringbuffer
RingBuffer::RingBuffer(int size)
{
  m_file = false;
  m_new = true;
  m_pathname = "";
  m_shmkey = IPC_PRIVATE;
  m_semkey = IPC_PRIVATE;

  openSHM(size);

  // Leave id of shm and semaphore in file name
  m_strbuf = new char[1024];
  sprintf(m_strbuf, "/tmp/SHM%d-SEM%d-UNNAMED", m_shmid, m_semid);
  int fd = open(m_strbuf, O_CREAT | O_TRUNC | O_RDWR, 0644);
  if (fd < 0) {
    B2WARNING("RingBuffer ID file could not be created.");
  } else {
    close(fd);
  }

  B2INFO("RingBuffer initialization done");
}

// Constructor of Global Ringbuffer with name
RingBuffer::RingBuffer(const char* name, unsigned int size)
{
  // 0. Determine shared memory type
  if (strcmp(name, "private") != 0) {      // Global
    m_file = true;
    m_pathname = string("/tmp/") + string(getenv("USER"))
                 + string("_RB_") + string(name);
    //m_pathfd = creat ( m_pathname.c_str(), 0644 );
    //    m_pathfd = open(m_pathname.c_str(), O_CREAT | O_EXCL, 0644);
    m_pathfd = open(m_pathname.c_str(), O_CREAT | O_EXCL | O_RDWR, 0644);
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

  openSHM(size);

  if (m_pathfd > 0) {
    B2INFO("First global RingBuffer creation: writing SHM info to file.");
    char rbufinfo[256];
    sprintf(rbufinfo, "%d\n", m_shmid);
    int is = write(m_pathfd, rbufinfo, strlen(rbufinfo));
    if (is < 0) perror("write");
    sprintf(rbufinfo, "%d\n", m_semid);
    is = write(m_pathfd, rbufinfo, strlen(rbufinfo));
    if (is < 0) perror("write");
    close(m_pathfd);
  }

  // Leave id of shm and semaphore in file name
  if (m_new) {
    m_strbuf = new char[1024];
    sprintf(m_strbuf, "/tmp/SHM%d-SEM%d-RB_%s", m_shmid, m_semid, name);
    int fd = open(m_strbuf, O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd < 0) {
      B2WARNING("RingBuffer ID file could not be created.");
    } else {
      close(fd);
    }
  }

  B2INFO("RingBuffer initialization done with shm=" << m_shmid);
}

RingBuffer::~RingBuffer()
{
  cleanup();
}

void RingBuffer::openSHM(int size)
{
  // 1. Open shared memory
  unsigned int sizeBytes = size * sizeof(int);
  m_shmid = shmget(m_shmkey, sizeBytes, IPC_CREAT | 0644);
  if (m_shmid < 0) {
    //    perror ( "RingBuffer:shmget" );
    unsigned int maxSizeBytes = size;
    ifstream shmax("/proc/sys/kernel/shmmax");
    if (shmax.good())
      shmax >> maxSizeBytes;
    shmax.close();

    const unsigned int oldSizeBytes = sizeBytes;
    size = maxSizeBytes / sizeof(int);
    sizeBytes = size * sizeof(int);
    B2WARNING("RingBuffer: shmget(" << oldSizeBytes << ") failed, limiting to system maximum: " << sizeBytes);
    m_shmid = shmget(IPC_PRIVATE, sizeBytes, IPC_CREAT | 0600);
    if (m_shmid < 0) {
      B2FATAL("RingBuffer: shmget(" << size * sizeof(int) << ") failed. Most likely the system doesn't allow us to reserve the needed shared memory. Try 'echo 500000000 > /proc/sys/kernel/shmmax' as root to set a higher limit (500MB).");
      return;
    }
  }
  m_shmadr = (int*) shmat(m_shmid, 0, 0);
  if (m_shmadr == (int*) - 1) {
    B2FATAL("RingBuffer::shmat() failed");
    return;
  }

  // 2. Open Semaphore
  m_semid = SemaphoreLocker::create(m_semkey);
  SemaphoreLocker locker(m_semid); //prevent simultaneous initialization

  // 3. Initialize control parameters
  m_shmsize = size;
  m_bufinfo = (struct RingBufInfo*) m_shmadr;
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
    m_bufinfo->mode = 0;
    m_bufinfo->ninsq = 0;
    m_bufinfo->nremq = 0;
  } else {
    m_bufinfo->nattached++;

    B2INFO("[RingBuffer] check entries = " << m_bufinfo->nbuf);
    B2INFO("[RingBuffer] check size = " << m_bufinfo->size);
  }

  m_remq_counter = 0;
  m_insq_counter = 0;

  B2DEBUG(100, "buftop = " << m_buftop << ", end = " << (m_buftop + m_bufinfo->size));
}

void RingBuffer::cleanup()
{
  shmdt((char*)m_shmadr);
  B2INFO("RingBuffer: Cleaning up IPC");
  if (m_new) {
    shmctl(m_shmid, IPC_RMID, (struct shmid_ds*) 0);
    SemaphoreLocker::destroy(m_semid);
    if (m_file) {
      //      close(m_pathfd);
      unlink(m_pathname.c_str());
    }
    unlink(m_strbuf);
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
  //  printf ( "insq: requesting : %d, nbuf = %d\n", size, m_bufinfo->nbuf );
  if (size <= 0) {
    printf("RingBuffer::insq : buffer size = %d, not queued.\n", size);
    return -1;
  }
  SemaphoreLocker locker(m_semid);
  if (m_bufinfo->nbuf == 0) {
    m_bufinfo->wptr = 0;
    m_bufinfo->rptr = 0;
    if (size > m_bufinfo->size + 2) {
      B2ERROR("[RingBuffer::insq ()] Inserted item is too large! (" << m_bufinfo->size + 2 << " < " << size << ")");
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
    return size;
  } else if (m_bufinfo->wptr > m_bufinfo->rptr) {
    if (m_bufinfo->mode != 4 &&
        m_bufinfo->mode != 3 && m_bufinfo->mode != 0) {
      B2ERROR("insq: Error in mode 0; current=" << m_bufinfo->mode);
      return -1;
    }
    if (m_bufinfo->mode == 3) {
      //      printf ( "---> mode is 3, still remaining buffers\n" );
      B2INFO("[RingBuffer] mode 3");
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
      return size;
    } else {
      if (m_bufinfo->rptr >= size + 2) { // buffer full and wptr>rptr
        if (m_bufinfo->mode != 0) {
          B2ERROR("insq: Error in mode 1; current=" << m_bufinfo->mode);
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
      if (m_bufinfo->mode != 1 && m_bufinfo->mode != 2 &&
          m_bufinfo->mode != 3) {
        B2ERROR("insq: Error in mode 2; current=" << m_bufinfo->mode);
        return -1;
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
  if (m_bufinfo->nbuf <= 0) {
    return 0;
  }
  //  printf ( "remq : nbuf = %d\n", m_bufinfo->nbuf );
  int* r_ptr = m_buftop + m_bufinfo->rptr;
  int nw = *r_ptr;
  //  printf ( "RingBuffer : nw = %d\n", nw );
  if (nw <= 0) {
    printf("RingBuffer::remq : buffer size = %d, skipped\n", nw);
    printf("RingBuffer::remq : entries = %d\n", m_bufinfo->nbuf);
    return 0;
  }
  //  printf ( "remq : taking buf from %d(%d)\n", m_bufinfo->rptr, nw );
  if (buf)
    memcpy(buf, r_ptr + 2, nw * sizeof(int));
  m_bufinfo->rptr = *(r_ptr + 1);
  //  if ( *(r_ptr+1) < m_bufinfo->rptr )
  if (m_bufinfo->rptr == 0)
    m_bufinfo->mode = 4;
  m_bufinfo->nbuf--;
  //  printf ( "remq: next buf = %d, nbuf = %d\n", m_bufinfo->rptr, m_bufinfo->nbuf );
  m_bufinfo->nremq++;
  m_remq_counter++;
  return nw;
}

int RingBuffer::spyq(int* buf) const
{
  SemaphoreLocker locker(m_semid);
  if (m_bufinfo->nbuf <= 0) {
    return 0;
  }
  //  printf ( "remq : nbuf = %d\n", m_bufinfo->nbuf );
  int* r_ptr = m_buftop + m_bufinfo->rptr;
  int nw = *r_ptr;
  //  printf ( "RingBuffer : nw = %d\n", nw );
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

int RingBuffer::shmid() const
{
  return m_shmid;
}

void RingBuffer::DumpInfo()
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
  printf("mode = %d\n", m_bufinfo->mode);
  printf("ninsq = %d\n", m_bufinfo->ninsq);
  printf("nremq = %d\n", m_bufinfo->ninsq);
}

