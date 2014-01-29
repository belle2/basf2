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

#include "TMessage.h"
#include <sys/time.h>


/* EvtMessage structure
   word 0 : Number of words in this record
   word 1 : Type RECORD_TYPE
   word 2 : Decode time
   word 3-15 : Reserved
   word 16- : List of streamed objects
    Streamed object :
      word 1 : size of obj
      word 2- : streamd object
*/

namespace Belle2 {

  /** What type of message is this? */
  enum RECORD_TYPE { MSG_EVENT, MSG_BEGIN_RUN, MSG_END_RUN, MSG_TERMINATE, MSG_NORECORD };

  /*! Header structure of streamed object list */
  struct EvtHeader {
    int size; /**< Number of words in this record. */
    RECORD_TYPE rectype; /**< Type of message. */
    struct timeval timestamp; /**< Decode time. */
    int src; /**< source IP. */
    int dest; /**< destination IP. */
    int reserved[10]; /**< For future use. */
  };

  /*! Class to manage streamed object */
  class EvtMessage {

  public:
    /** maximal EvtMessage size, in bytes (50MB->200MB). */
    //    const static unsigned int c_MaxEventSize = 50000000;
    const static unsigned int c_MaxEventSize = 200000000;

    /*! build EvtMessage from existing buffer (does not take ownership). */
    EvtMessage(char* buf = NULL);
    /*! build EvtMessage by allocating new message buffer (sobjs is copied). */
    EvtMessage(const char* sobjs, int size, RECORD_TYPE type);
    /*! Copy constructor (m_data is copied). */
    EvtMessage(const EvtMessage& evtmsg);
    /*! Destructor */
    ~EvtMessage();

    /*! Assignment (m_data is copied). */
    EvtMessage& operator=(const EvtMessage& obj);


    // Access functions
    /*! Get buffer address
     *
     * If you own the EvtMessage object, the memory is guaranteed to be at
     * least sizeof(int) * paddedSize() bytes. Bytes exceeding size() are
     * zeroed.
     */
    char* buffer();
    /*! Set existing buffer address */
    void  buffer(const char*);

    /*! Get size of message including headers*/
    int   size();
    /** Same as size(), but as size of an integer array.
     *
     * Use this for passing EvtMessage to RingBuffer::insq().
     */
    int   paddedSize();
    /*! Get size of message body */
    int   msg_size();

    /*! Get record type */
    RECORD_TYPE type();
    /*! Set record type */
    void type(RECORD_TYPE);

    /*! Get time stamp */
    struct timeval time();
    /*! Set time stamp */
    void time(struct timeval& time);

    /*! Get source IP of message */
    int   src();
    /*! Set source IP of message */
    void  src(int src);

    /*! Get destination IP of message */
    int   dest();
    /*! Set destination IP of message */
    void  dest(int dest);

    /*! Get pointer to EvtHeader */
    EvtHeader* header();
    /*! Get pointer to message body */
    char* msg();
    /*! Copy message into newly allocated buffer */
    void msg(const char* msg, int size, RECORD_TYPE type);

  private:
    char* m_data;         ///< Pointer to the internal EvtMessage buffer
    bool m_ownsBuffer; ///< Wether to clean up m_data in destructor

  };

  /*!  Message class derived from TMessage */
  class InMessage : public TMessage {
  public:
    /*! Constructor to build a message */
    InMessage(void* buf, int len) : TMessage(buf, len) {
      //std::cout << "MSG BUILD: " << len << std::endl;
      //std::cout << "MSG: " << buf << std::endl;
      this->SetBit(kIsOwner, false);
    }
  };

}
#endif
