//+
// File : EvtMessage.h
// Description : Interface to RingBuffer and Socket
//
// Author : Soohyung Lee and Ryosuke Itoh
// Date : 24 - Jul - 2008
// Modified : 4 - Jun - 2010
//-
#ifndef EVTMESSAGE_H
#define EVTMESSAGE_H

#include <RtypesCore.h>

struct timeval;

namespace Belle2 {

  /** What type of message is this?
   *
   * Note: for parallel processing, the only type that can work is MSG_EVENT. Other messages would need to be sent once to each process, which would result in all kinds of race conditions.
   */
  enum RECORD_TYPE { MSG_EVENT, MSG_BEGIN_RUN, MSG_END_RUN, MSG_TERMINATE, MSG_NORECORD, MSG_STREAMERINFO };

  /** Header structure of streamed object list */
  struct EvtHeader {
    UInt_t size; /**< Number of words in this record. */
    RECORD_TYPE rectype; /**< Type of message. */
    Long64_t time_sec; /**< seconds part of timeval. */
    Long64_t time_usec; /**< seconds part of timeval. */
    UInt_t src; /**< source IP. */
    UInt_t dest; /**< destination IP. */
    UInt_t alsoReserved; /**< obsolete member removed, can be replaced with something else. */
    UInt_t nObjects; /**< #objects in message. */
    UInt_t nArrays; /**< #objects in message. */
    UInt_t reserved[7]; /**< Reserved for future use. Don't ever use these directly. */
  };

  /** Class to manage streamed object.
   *
   * Binary stream consists of:
   *  Fields in EvtHeader (see definition)
   *  List of streamed objects, each consisting of:
   *    word 1 : size of object
   *    word 2- : streamed object
   */
  class EvtMessage {

  public:
    /** maximal EvtMessage size, in bytes (200MB). */
    const static unsigned int c_MaxEventSize = 200000000;

    /** build EvtMessage from existing buffer (no copy, but does not take ownership). */
    explicit EvtMessage(char* buf = nullptr);
    /** build EvtMessage by allocating new message buffer (sobjs is copied). */
    EvtMessage(const char* sobjs, int size, RECORD_TYPE type);
    /** Copy constructor (m_data is copied). */
    EvtMessage(const EvtMessage& evtmsg);
    /** Destructor */
    ~EvtMessage();

    /** Assignment (m_data is copied). */
    EvtMessage& operator=(const EvtMessage& obj);


    // Access functions
    /** Get buffer address
     *
     * If you own the EvtMessage object, the memory is guaranteed to be at
     * least sizeof(int) * paddedSize() bytes. Bytes exceeding size() are
     * zeroed.
     */
    char* buffer();
    /** Set existing buffer address (copies buffer) */
    void  buffer(const char*);

    /** Get size of message including headers. Often, you'll want to use paddedSize() instead. */
    int   size() const;
    /** Same as size(), but as size of an integer array.
     *
     * Use this for passing EvtMessage to RingBuffer::insq().
     */
    int   paddedSize() const;
    /** Get size of message body */
    int   msg_size() const;

    /** Get record type */
    RECORD_TYPE type() const;
    /** Set record type */
    void type(RECORD_TYPE);

    /** Get time stamp */
    struct timeval time() const;
    /** Set time stamp */
    void setTime(const struct timeval& time);

    /** Get source IP of message */
    int   src() const;
    /** Set source IP of message */
    void  src(int src);

    /** Get destination IP of message */
    int   dest() const;
    /** Set destination IP of message */
    void  dest(int dest);

    /** Get pointer to EvtHeader */
    EvtHeader* header();
    /** Get pointer to message body */
    char* msg();

  private:
    /** Copy message into newly allocated buffer */
    void setMsg(const char* msg, int size, RECORD_TYPE type);

    char* m_data;         ///< Pointer to the internal EvtMessage buffer
    bool m_ownsBuffer; ///< Wether to clean up m_data in destructor
  };
}
#endif
