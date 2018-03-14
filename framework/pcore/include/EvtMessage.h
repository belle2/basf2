//+
// File : EvtMessage.h
// Description : Interface to RingBuffer and Socket
//
// Author : Soohyung Lee and Ryosuke Itoh
// Date : 24 - Jul - 2008
// Modified : 4 - Jun - 2010
//-

#pragma once

#include <RtypesCore.h>

struct timeval;

namespace Belle2 {

  /** What type of message is this?
   *
   * Note: for parallel processing, the only type that can work is MSG_EVENT. Other messages would need to be sent once to each process, which would result in all kinds of race conditions.
   */
  enum ERecordType { MSG_EVENT, MSG_BEGIN_RUN, MSG_END_RUN, MSG_TERMINATE, MSG_NORECORD, MSG_STREAMERINFO };

  /** Header structure of streamed object list */
  struct EvtHeader {
    /** set number of words and record type. */
    EvtHeader(UInt_t aSize, ERecordType aRectype): size(aSize), rectype(aRectype) {}
    UInt_t size; /**< Number of words in this record. */
    ERecordType rectype; /**< Type of message. */
    Long64_t time_sec{0}; /**< seconds part of timeval. */
    Long64_t time_usec{0}; /**< micro seconds part of timeval. */
    UInt_t src{(UInt_t) - 1}; /**< source IP. */
    UInt_t dest{(UInt_t) - 1}; /**< destination IP. */
    UInt_t flags{0}; /**< flags concerning the content of the message. Usually 0
                      but can be any combination of of EvtMessage::EMessageFlags. */
    UInt_t nObjects{0}; /**< number of objects in message. */
    UInt_t nArrays{0}; /**< number of objects in message. */
    UInt_t reserved[6] {0}; /**< Reserved for future use. Don't ever use these directly. */
    /** version field. Previously the reserved fields were not initialized
     * properly so they could contain random garbage which makes it very hard
     * to check for anything. Now we send 0xBEEFED + 8bit version to indicate
     * that yes, this is indeed a valid version and not just random garbage.
     * Chance of collision is low but there is nothing else we can do except
     * breaking compatibility with old files. */
    UInt_t version{0xBEEFED01};
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

    /** Flags for the message */
    enum EMessageFlags {
      /** indicates that the message body is compressed and should be
       * uncompressed using ROOT R__unzip_header and R__unzip before use */
      c_MsgCompressed = 1
    };

    /** build EvtMessage from existing buffer (no copy, but does not take ownership). */
    explicit EvtMessage(char* buf = nullptr);
    /** build EvtMessage by allocating new message buffer (sobjs is copied). */
    EvtMessage(const char* msg, int size, ERecordType type);
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

    /** get version of the header. Returns 0 for no valid version information */
    unsigned int getVersion() const { return ((getHeader()->version & 0xFFFFFF00) == 0xBEEFED00) ? (getHeader()->version & 0xFF) : 0;}
    /** Get flags of the  message */
    unsigned int getMsgFlags() const { return getVersion() > 0 ? getHeader()->flags : 0; }
    /** Set flags for the message */
    void setMsgFlags(unsigned int flags) { header()->flags = flags;}
    /** Add flags to the  message */
    void addMsgFlags(unsigned int flags) { header()->flags |= flags;}
    /** Check if the message has the given flags */
    bool hasMsgFlags(unsigned int flags) const { return (getMsgFlags() & flags) == flags; }

    /** Get record type */
    ERecordType type() const;
    /** Set record type */
    void type(ERecordType);

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
    /** Get pointer to EvtHeader */
    const EvtHeader* getHeader() const;
    /** Get pointer to message body */
    char* msg();

  private:
    /** Copy message into newly allocated buffer */
    void setMsg(const char* msg, int size, ERecordType type);

    char* m_data;         ///< Pointer to the internal EvtMessage buffer
    bool m_ownsBuffer; ///< Wether to clean up m_data in destructor
  };
}
