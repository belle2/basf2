/// @file RingBuffer.h
/// @brief Ring buffer definition
/// @author Ryosuke Itoh
/// @date Feb 22 2010

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <sys/types.h>
#include <string>

namespace Belle2 {

  /*! A structure to manage ring buffer. Placed on top of the shared memory. */
  struct RingBufInfo {
    int size; /**< ring buffer size, minus this header. */
    int remain; /**< Unsure, always equal to size. */
    int wptr; /**< Pointer for writing entries. */
    int prevwptr; /**< Previous state of wptr (for error recovery). */
    int rptr; /**< Pointer for reading entries. */
    int nbuf; /**< Number of entries in ring buffer. */
    int semid; /**< Semaphore ID. */
    int nattached; /**< Number of RingBuffer instances currently attached to this buffer. */
    int redzone; /**< Unused. */
    int readbuf; /**< Unused. */
    int mode; /**< Error state? 0: Normal, 1: buffer full and wptr>rptr, others are complicated. */
    int msgid; /**< Unused. */
    int ninsq; /**< Count insq() calls for this buffer. */
    int nremq; /**< Count remq() calls for this buffer. */
  };

  /*! Class to manage a Ring Buffer placed in an IPC shared memory */
  class RingBuffer {
  public:
    /** Standard size of buffer, in integers (~40MB). */
    const static int c_DefaultSize = 10000000;

    /** Constructor to create a new shared memory in private space.
     *
     * @param size Ring buffer size in integers (!)
     */
    RingBuffer(int size = c_DefaultSize);
    /*! Constructor to create/attach named shared memory in global space */
    RingBuffer(const char* name, unsigned int size = 0);     // Create / Attach Ring buffer
    /*! Constructor by attaching to an existing shared memory */
    //    RingBuffer(int shmid);              // Attach Ring Buffer
    /*! Destructor */
    ~RingBuffer();
    /** open shared memory */
    void openSHM(int size);
    /*! Function to detach and remove shared memory*/
    void cleanup();

    /*! Append a buffer to the RingBuffer */
    int insq(const int* buf, int size);
    /*! Pick up a buffer from the RingBuffer */
    int remq(int* buf);
    /*! Prefetch a buffer from the RingBuffer w/o removing it*/
    int spyq(int* buf) const;
    /*! Returns number of entries/buffers in the RingBuffer */
    int numq() const;
    /*! Clear the RingBuffer */
    int clear();

    /*! Return ID of the shared memory */
    int shmid() const;

    // Debugging functions
    /** Print some info on the RingBufInfo structure. */
    void dump_db();
    /** Return number of insq() calls for current buffer. */
    int ninsq() const;
    /** Return number of remq() calls for current buffer. */
    int nremq() const;

    /** Return number of insq() calls. */
    int insq_counter() const;
    /** Return number of remq() calls. */
    int remq_counter() const;

    /** Dump buffer info */
    void DumpInfo();

  private:
    bool m_new; /**< True if we created the ring buffer ourselves (and need to clean it). */
    bool m_file; /**< True if m_pathfd needs to be closed. */
    std::string m_pathname; /**< Path for identifying shared memory if named ring buffer is created. */
    int  m_pathfd; /**< Associated file descriptor. */
    key_t m_shmkey; /**< SHM key, see shmget(2). */
    key_t m_semkey; /**< Semaphore key, see semget(2). */
    char* m_strbuf; /**< file name containing ids of shm and sema for private shared mem */

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
