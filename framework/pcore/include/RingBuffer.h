/// @file RingBuffer.h
/// @brief Ring buffer definition
/// @author Ryosuke Itoh
/// @date Feb 22 2010

#pragma once

#include <sys/types.h>
#include <string>

namespace Belle2 {

  /** Internal metadata structure for RingBuffer. Placed on top of the shared memory. */
  struct RingBufInfo {
    int size; /**< ring buffer size (integers), minus this header. */
    int remain; /**< Unsure, always equal to size. */
    int wptr; /**< Pointer for writing entries. */
    int prevwptr; /**< Previous state of wptr (for error recovery). */
    int rptr; /**< Pointer for reading entries. */
    int nbuf; /**< Number of entries in ring buffer. */
    int semid; /**< Semaphore ID. */
    int nattached; /**< Number of RingBuffer instances currently attached to this buffer. */
    int nbusy; /**< Number of attached _reading_ processes currently processing events. */
    int __readbuf; /**< Unused. */
    int errtype; /**< Error state? 0: Normal, 1: buffer full and wptr>rptr, others are complicated. */
    int numAttachedTx; /**< number of attached sending processes. 0: Processes reading from this buffer should terminate once it's empty. -1: attach pending (initial state) */
    int ninsq; /**< Count insq() calls for this buffer. */
    int nremq; /**< Count remq() calls for this buffer. */
  };

  /** Class to manage a Ring Buffer placed in an IPC shared memory */
  class RingBuffer {
  public:
    /** Standard size of buffer, in integers (~60MB). Needs to be large enough to contain any event, but adds to total memory use of basf2. */
    const static int c_DefaultSize = 15000000;

    /** Constructor to create a new shared memory in private space.
     *
     * @param nwords Ring buffer size in integers
     */
    explicit RingBuffer(int nwords = c_DefaultSize);
    /** Constructor to create/attach named shared memory in global space */
    RingBuffer(const std::string& name, unsigned int nwords = 0);     // Create / Attach Ring buffer
    /** Destructor */
    ~RingBuffer();
    /** open shared memory */
    void openSHM(int nwords);
    /** Function to detach and remove shared memory*/
    void cleanup();

    /** Append a buffer to the RingBuffer */
    int insq(const int* buf, int size);
    /** Pick up a buffer from the RingBuffer */
    int remq(int* buf);
    /** Prefetch a buffer from the RingBuffer w/o removing it*/
    int spyq(int* buf) const;
    /** Returns number of entries/buffers in the RingBuffer */
    int numq() const;

    /** Increase the number of attached Tx counter. */
    void txAttached();
    /** Decrease the number of attached Tx counter. */
    void txDetached();
    /** Cause termination of reading processes (if they use isDead()). Assumed to be atomic. */
    void kill();

    /** If True, the ring buffer is empty and has no attached Tx modules (i.e. no new data is going to be added). Processes should then stop. */
    bool isDead() const;
    /** True if and only if buffer is empty and nbusy == 0.
     *
     * Called in Tx to see if all events of the current run
     * have been processed */
    bool allRxWaiting() const;

    /** Clear the RingBuffer */
    int clear();
    /** Forcefully clear the RingBuffer with resetting semaphore*/
    void forceClear();

    /** Clear the RingBuffer, if the semaphore isn't locked at the moment. See SemaphoreLocker::isLocked() for details */
    int tryClear();

    /** Return ID of the shared memory */
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

    /** Dump contents of RingBufInfo metadata */
    void dumpInfo() const;

  private:
    bool m_new; /**< True if we created the ring buffer ourselves (and need to clean it). */
    bool m_file; /**< True if m_pathfd needs to be closed. */
    std::string m_pathname; /**< Path for identifying shared memory if named ring buffer is created. */
    int  m_pathfd; /**< Associated file descriptor. */
    key_t m_shmkey; /**< SHM key, see shmget(2). */
    key_t m_semkey; /**< Semaphore key, see semget(2). */
    /** file path containing ids of shm and sema for private shared mem, used for easier cleanup if we fail to do things properly */
    std::string m_semshmFileName;

    /** Is this process currently processing events from this RingBuffer?
     *
     * set during remq() with value depending on wether data was returned.
     * Always false for a process that is only using insq().
     */
    bool m_procIsBusy;

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
