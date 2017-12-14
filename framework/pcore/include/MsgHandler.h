//+
// File : MsgHandler.cc
// Description : Encode/Decode EvtMessage
//
// Author : SooHyung Lee, Ryosuke Itoh
// Date : 31 - Jul - 2008
// Modified : 4 - Jun - 2010
//-

#pragma once

#include <framework/pcore/EvtMessage.h>
#include <TMessage.h>

#include <string>
#include <memory>

class TObject;

namespace Belle2 {
  /** dynamic character buffer that knows its size.
   *
   * This is similar to std::vector<char> but compared with std::vector<char>
   * this saves unnecessary calls to memset() when adding a block. (std::vector
   * initialises memory on allocation, this class does not).
   */
  class CharBuffer {
    std::unique_ptr<char[]> m_data; /**< data buffer. */
    size_t m_capacity{0}; /**< size of allocated memory in m_data */
    size_t m_size{0}; /**< current size, <= m_capacity */
  public:
    /** Constructor, with the initial capacity of the buffer to allocate
     * (in bytes). size() will remain zero until buffer is filled.
     */
    explicit CharBuffer(size_t initial_capacity = 0): m_data{initial_capacity > 0 ? new char[initial_capacity] : nullptr}
    {}
    /** copy data to end of buffer, expanding buffer if needed.
     * This can invalidate previous pointers obtained by data() if the buffer
     * needs to be expanded. */
    void add(const void* data, size_t len)
    {
      auto old_size = m_size;
      resize(m_size + len);
      memcpy(m_data.get() + old_size, data, len);
    }
    /** return raw pointer.
     * Should not be read beyond size() bytes and becomes invalid on add() or
     * resize() */
    char* data() { return m_data.get(); }
    /** return buffer size (do not access data() beyond this) */
    size_t size() const { return m_size; }
    /** reset (without deallocating) */
    void clear() { m_size = 0; }
    /** resize, similar to std::vector<char>::resize in that it will copy the
     * existing buffer to a new, larger buffer if needed but it will not
     * initialize any elements beyond the existing elements */
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
    /** Constructor
     * @param complevel Compression level and algorithm: algorithm*100 + level where algorithm can be one of
     *   0: default root system algorithm (usually zlib)
     *   1: zlib
     *   2: lzma
     *   3: old root compression code
     *   and level can be between 1 and 9 (0 disables compression). So for
     *   example 101 corresponds to zlib with minimal compression while 209
     *   means lzma with maximal compression.
     */
    explicit MsgHandler(int complevel = 0);
    /** Destructor */
    virtual ~MsgHandler();

    /** Clear object list */
    virtual void clear();
    /** Add an object to be streamed */
    virtual void add(const TObject*, const std::string& name);

    /** Stream object list into an EvtMessage. Caller is responsible for deletion. */
    virtual EvtMessage* encode_msg(ERecordType rectype);
    /** Decode an EvtMessage into a vector list of objects with names */
    virtual void decode_msg(EvtMessage* msg, std::vector<TObject*>& objlist, std::vector<std::string>& namelist);

  private:
    CharBuffer m_buf; /**< EvtMessage character buffer for encode_msg(). */
    CharBuffer m_compBuf; /**< EvtMessage character buffer for compressing/decompressing. */
    std::unique_ptr<TMessage> m_msg; /**< Used for serialising objects into m_buf. */
    InMessage m_inMsg; /**< Used for deserializing in decode_msg() */
    int m_complevel; /**< compression algorithm * 100 + compression level.
                      level can be 0 for no compression to 9 for highes
                      compression, algorithm can be one of default (0), zlib
                      (1) or lzma (2) */
  };

} // namespace Belle2
