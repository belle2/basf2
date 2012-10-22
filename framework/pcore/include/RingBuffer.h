/// @file ring_bugger.h
/// @brief Ring buffer definition
/// @author Ryosuke Itoh
/// @date Feb 22 2010

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <iostream>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <boost/format.hpp>

namespace Belle2 {

  /*! A structure to manage ring buffer. Placed on top of the shared memory. */
  struct RingBufInfo {
    int size; /**< ring buffer size, minus this header. */
    int remain;
    int wptr;
    int prevwptr;
    int rptr;
    int nbuf;
    int semid; /**< Semaphore ID. */
    int nattached; /**< Number of RingBuffer instances currently attached to this buffer. */
    int redzone;
    int readbuf;
    int mode;
    int msgid;
    int ninsq; /**< Count insq() calls for this buffer. */
    int nremq; /**< Count remq() calls for this buffer. */
  };

  /*! Class to manage a Ring Buffer placed in an IPC shared memory */
  class RingBuffer {
  public:
    /*! Constructor to create a new shared memory in private space */
    RingBuffer(int size);
    /*! Constructor to create/attach named shared memory in global space */
    RingBuffer(const char* name, unsigned int size);    // Create / Attach Ring buffer
    /*! Constructor by attaching to an existing shared memory */
    //    RingBuffer(int shmid);              // Attach Ring Buffer
    /*! Destructor */
    ~RingBuffer();
    /*! Function to detach and remove shared memory*/
    void cleanup(void);

    /*! Append a buffer in the RingBuffer */
    int insq(int* buf, int size);
    /*! Pick up a buffer from the RingBuffer */
    int remq(int* buf);
    /*! Prefetch a buffer from the RingBuffer w/o removing it*/
    int spyq(int* buf);
    /*! Returns number of buffers in the RingBuffer */
    int numq(void);
    /*! Clear the RingBuffer */
    int clear(void);

    /*! Return ID of the shared memory */
    int shmid(void);

    // Debugging functions
    void dump_db(void);
    int ninsq(void);
    int nremq(void);

    /** Return number of insq() calls. */
    int insq_counter(void);
    /** Return number of remq() calls. */
    int remq_counter(void);

  private:
    /** Lock the given semaphore. */
    int sem_lock(int);
    /** Unlock the given semaphore. */
    int sem_unlock(int);

  private:
    bool m_new; /**< True if we created the ring buffer ourselves (and need to clean it). */
    bool m_file;
    std::string m_pathname;
    int  m_pathfd; /** Associated file descriptor. */
    key_t m_shmkey; /**< SHM key, see shmget(2). */
    key_t m_semkey; /**< Semaphore key, see semget(2). */

    int  m_shmid; /**< ID of shared memory segment. (See shmget(2)) */
    int* m_shmadr; /**< Address of attached shared memory segment. (See shmat(2)) */
    int  m_shmsize; /**< Size of shared memory segment, in bytes. */
    struct RingBufInfo* m_bufinfo; /**< structure to manage ring buffer. Placed on top of the shared memory. */
    int* m_buftop; /**< Points to memory after the end of m_bufinfo. */
    int  m_semid; /**< Semaphore ID. */
    int  m_remq_counter; /**< count remq() calls. */
    int  m_insq_counter; /**< count insq() calls. */
  };

}
#endif
