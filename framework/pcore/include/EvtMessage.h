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
#include "framework/pcore/RingBuffer.h"


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
#define MSG_HEADER_SIZE  16

namespace Belle2 {

  enum RECORD_TYPE { MSG_EVENT, MSG_BEGIN_RUN, MSG_END_RUN, MSG_TERMINATE, MSG_NORECORD };

  /*! Header structure of streamed object list */
  struct EvtHeader {
    int size;
    RECORD_TYPE rectype;
    struct timeval timestamp;
    int src;
    int dest;
    int reserved[10];
  };

  /*! Class to manage streamed object */
  class EvtMessage {

  public:
    /*! Empty constructor */
    EvtMessage(void);
    /*! build EvtMessage from existing buffer */
    EvtMessage(char* buf);
    /*! build EvtMessage by allocating new message buffer */
    EvtMessage(char* sobjs, int size, RECORD_TYPE type);
    /*! Copy constructor */
    EvtMessage(EvtMessage& evtmsg);
    /*! Destructor */
    ~EvtMessage(void);

    /*! Operator */
    EvtMessage& operator=(EvtMessage& obj);


    // Access functions
    /*! Get buffer address */
    char* buffer(void);
    /*! Set existing buffer address */
    void  buffer(char*);

    /*! Get size of message including headers*/
    int   size(void);
    /*! Get size of message body */
    int   msg_size(void);

    /*! Get record type */
    RECORD_TYPE type(void);
    /*! Set record type */
    void type(RECORD_TYPE);

    /*! Get time stamp */
    struct timeval time(void);
    /*! Set time stamp */
    void time(struct timeval& time);

    /*! Get source IP of message */
    int   src(void);
    /*! Set source IP of message */
    void  src(int src);

    /*! Get destination IP of message */
    int   dest(void);
    /*! Set destination IP of message */
    void  dest(int dest);

    /*! Get pointer to EvtHeader */
    EvtHeader* header(void);
    /*! Get pointer to message body */
    char* msg(void);
    /*! Copy message into newly allocated buffer */
    void msg(const char* msg, int size, RECORD_TYPE type);

  private:
    char* m_data;         ///< Pointer to the internal EvtMessage buffer
    int   m_buftype;

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

} // namespace Roobasf

#endif
