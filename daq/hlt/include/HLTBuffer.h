/// @file HLT_bugger.h
/// @brief HLT buffer definition
/// @author Ryosuke Itoh
/// @date Feb 22 2010

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>

#ifndef HLTBUFFER_H
#define HLTBUFFER_H

namespace Belle2 {

  /*! A structure to manage HLT buffer. Placed on top of the shared memory. */
  struct HLTBufInfo {
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

  /*! Class to manage a HLT Buffer placed in an IPC shared memory */
  class HLTBuffer {
  public:
    /*! Constructor by creating a new shared memory */
    HLTBuffer(const char* name, int size);    // Create / Attach HLT buffer
    /*! Constructor by attaching to an existing shared memory */
    HLTBuffer(int shmid);              // Attach HLT Buffer
    /*! Destructor */
    ~HLTBuffer();
    /*! Function to detach and remove shared memory*/
    void cleanup(void);

    /*! Append a buffer in the HLTBuffer */
    int insq(int* buf, int size);
    /*! Pick up a buffer from the HLTBuffer */
    int remq(int* buf);
    /*! Returns number of buffers in the HLTBuffer */
    int numq(void);
    /*! Clear the HLTBuffer */
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
    int  m_shmid;
    int* m_shmadr;
    int  m_shmsize;
    struct HLTBufInfo* m_bufinfo;
    int* m_buftop;
    int  m_semid;
    int  m_msgid;
    int  m_remq_counter;
    int  m_insq_counter;
  };

} // namespace Roobasf


#endif



