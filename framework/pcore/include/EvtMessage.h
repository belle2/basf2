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

// Header structure
  struct EvtHeader {
    int size;
    RECORD_TYPE rectype;
    struct timeval timestamp;
    int src;
    int dest;
    int reserved[10];
  };

  class EvtMessage {

  public:
    // Empty constructor
    EvtMessage(void);
    // build EvtMessage from existing buffer
    EvtMessage(char* buf);
    // build EvtMessage by allocating new message buffer
    EvtMessage(char* sobjs, int size, RECORD_TYPE type);
    // Copy constructor
    EvtMessage(EvtMessage& evtmsg);
    // Destructor
    ~EvtMessage(void);

    // Operator
    EvtMessage& operator=(EvtMessage& obj);


    // Access functions
    char* buffer(void);
    void  buffer(char*);

    int   size(void);
    int   msg_size(void);

    RECORD_TYPE type(void);
    void type(RECORD_TYPE);

    struct timeval time(void);
    void time(struct timeval& time);

    int   src(void);
    void  src(int src);

    int   dest(void);
    void  dest(int dest);

    EvtHeader* header(void);
    char* msg(void);
    void msg(const char* msg, int size, RECORD_TYPE type);

  private:
    char* m_data;         ///< Pointer to the internal EvtMessage buffer
    int   m_buftype;

  };

/// @class InMessage
/// @brief Inherited message class to get TMessage
  class InMessage : public TMessage {
  public:
    InMessage(void* buf, int len) : TMessage(buf, len) {
      //std::cout << "MSG BUILD: " << len << std::endl;
      //std::cout << "MSG: " << buf << std::endl;
      this->SetBit(kIsOwner, false);
    }
  };

} // namespace Roobasf

#endif
