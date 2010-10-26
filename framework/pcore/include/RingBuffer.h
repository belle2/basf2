/// @file ring_bugger.h
/// @brief Ring buffer definition
/// @author Ryosuke Itoh
/// @date Feb 22 2010

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

namespace Belle2 {


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

  class RingBuffer {
  public:
    RingBuffer(char* name, int size);    // Create / Attach Ring buffer
    RingBuffer(int shmid);              // Attach Ring Buffer
    ~RingBuffer();
    void cleanup(void);

    int insq(int* buf, int size);
    int remq(int* buf);
    int numq(void);
    int clear(void);

    int shmid(void);

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
    struct RingBufInfo* m_bufinfo;
    int* m_buftop;
    int  m_semid;
    int  m_msgid;
    int  m_remq_counter;
    int  m_insq_counter;
  };

} // namespace Roobasf


#endif



