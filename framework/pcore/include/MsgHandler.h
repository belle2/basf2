//+
// File : MsgHandler.cc
// Description : Encode/Decode EvtMessage
//
// Author : SooHyung Lee, Ryosuke Itoh
// Date : 31 - Jul - 2008
// Modified : 4 - Jun - 2010
//-

#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include <framework/pcore/EvtMessage.h>
#include <TMessage.h>

#include <string>
#include <memory>

class TObject;
class TMessage;

namespace Belle2 {
  /** dynamic character buffer that knows its size.
   *
   * This is similar to std::vector<char> but compared to std::vector we are
   * not initializing the memory area before using it which saves some time.
   */
  class CharBuffer {
    std::unique_ptr<char[]> m_data; /**< data buffer. */
    size_t m_capacity{0}; /**< allocated size */
    size_t m_size{0}; /**< current size, <= m_vec.size() */
  public:
    /** Constructor, with the initial size of the buffer to allocate
     * (in bytes). size() will remain zero until buffer is filled.
     */
    explicit CharBuffer(size_t initial_capacity = 0): m_data{initial_capacity > 0 ? new char[initial_capacity] : nullptr}
    {}
    /** copy data to end of buffer. */
    void add(const void* data, size_t len)
    {
      auto old_size = m_size;
      resize(m_size + len);
      memcpy(m_data.get() + old_size, data, len);
    }
    /** return raw pointer. */
    char* data() { return m_data.get(); }
    /** return buffer size (do not access data() beyond this) */
    size_t size() const { return m_size; }
    /** reset (without deallocating) */
    void clear() { m_size = 0; }
    /** resize, similar to std::vector<char>::resize in that it will copy the
     * existing buffer to a new, larger buffer if needed but it will not
     * initialize any elements beyond this point */
    void resize(size_t size)
    {
      if (size > m_capacity) {
        // grow by basically doubling each time, except if the size we need to
        // grow to is even more than that
        m_capacity = std::max(size, m_capacity * 2);
        //c++14: auto newbuf = std::make_unique<char[]>(m_capacity);
        std::unique_ptr<char[]> newbuf{new char[m_capacity]};
        memcpy(newbuf.get(), m_data.get(), m_size);
        std::swap(m_data, newbuf);
      }
      m_size = size;
    }
  };

  /** Reusable Message class derived from TMessage (for reading only) */
  class InMessage : public TMessage {
  public:
    InMessage() : TMessage()
    {
      SetReadMode();
      SetWhat(kMESS_OBJECT);
    }

    /** Replace buffer (doesn't take ownership). */
    void SetBuffer(const void* ptr, UInt_t bufsize)
    {
      TBuffer::SetBuffer(const_cast<void*>(ptr), bufsize, false);
      // TMessage has an extra header of two ints: a reserved size field and a
      // fWhat pointer indicating the type of the message.  We force the message
      // to be MESS_OBJECT so we don't care. Let's put the buffer where we need
      // it to be. The original constructor has a ReadClass() here but we skip
      // it since we read everything as TObject.
      SetBufferOffset(sizeof(UInt_t) * 2);
      // and reset the map of objects/classes seen so far
      ResetMap();
    }

    /** Read one object from the message assuming it inherits from TObject */
    TObject* readTObject() { return static_cast<TObject*>(ReadObjectAny(TObject::Class())); }
  };

  /** A class to encode/decode an EvtMessage */
  class MsgHandler {
  public:
    /** Constructor */
    explicit MsgHandler(int complevel = 0);
    /** Destructor */
    virtual ~MsgHandler();

    /** Clear object list */
    virtual void clear();
    /** Add an object to be streamed */
    virtual void add(const TObject*, const std::string& name);

    /** Stream object list into an EvtMessage. Caller is responsible for deletion. */
    virtual EvtMessage* encode_msg(RECORD_TYPE rectype);
    /** Decode an EvtMessage into a vector list of objects with names */
    virtual void decode_msg(EvtMessage* msg, std::vector<TObject*>& objlist, std::vector<std::string>& namelist);

  private:
    std::unique_ptr<CharBuffer> m_buf; /**< EvtMessage character buffer for encode_msg(). */
    std::unique_ptr<TMessage> m_msg; /**< Used for serialising objects into m_buf. */
    InMessage m_inMsg; /**< Used for deserializing in decode_msg() */
    int m_complevel; /**< compression level, from 0 (none) to 9 (highest). */
  };

} // namespace Belle2

#endif
