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

#include <vector>
#include <string>
#include <memory>

class TObject;
class TMessage;

namespace Belle2 {
  /** dynamic character buffer that knows its size.
   *
   * compared with std::vector<char> this saves some allocations if reused.
   */
  class CharBuffer {
    std::vector<char> m_vec; /**< data buffer. */
    size_t m_size = 0; /**< current size, <= m_vec.size() */
  public:
    CharBuffer(size_t initial_size = 0)
    {
      m_vec.resize(initial_size);
    }
    /** copy data to end of buffer. */
    void add(const void* data, size_t len)
    {
      if (m_size + len > m_vec.size()) {
        m_vec.resize(m_size + len);
      }
      memcpy(m_vec.data() + m_size, data, len);
      m_size += len;
    }
    /** return raw pointer. */
    char* data() { return m_vec.data(); }
    /** return buffer size (do not access data() beyond this) */
    size_t size() const { return m_size; }
    /** reset (without deallocating) */
    void clear() { m_size = 0; }
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
    virtual bool add(const TObject*, const std::string& name);

    /** Stream object list into an EvtMessage. Caller is responsible for deletion. */
    virtual EvtMessage* encode_msg(RECORD_TYPE rectype);
    /** Decode an EvtMessage into a vector list of objects with names */
    virtual int decode_msg(EvtMessage* msg, std::vector<TObject*>& objlist, std::vector<std::string>& namelist);

  private:
    std::unique_ptr<CharBuffer> m_buf; /**< EvtMessage character buffer for encode_msg(). */
    std::unique_ptr<TMessage> m_msg; /**< Used for serialising objects into m_buf. */
    int m_complevel; /**< compression level, from 0 (none) to 9 (highest). */
  };

} // namespace Belle2

#endif
