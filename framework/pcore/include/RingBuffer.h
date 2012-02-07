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
    int size;
    int remain;
    int wptr;
    int prevwptr;
    int rptr;
    int nbuf;
    int semid;
    int nattached;
    int redzone;
    int readbuf;
    int mode;
    int msgid;
    int ninsq;
    int nremq;
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

    int insq_counter(void);
    int remq_counter(void);

  private:
    int compress(void);
    int sem_lock(int);
    int sem_unlock(int);

  private:
    bool m_new;
    bool m_file;
    std::string m_pathname;
    int  m_pathfd;
    key_t m_shmkey;
    key_t m_semkey;

    int  m_shmid;
    int* m_shmadr;
    int  m_shmsize;
    struct RingBufInfo* m_bufinfo;
    int* m_buftop;
    int  m_semid;
    int  m_msgid;
    int  m_remq_counter;
    int  m_insq_counter;
  };

} // namespace Roobasf


#endif



