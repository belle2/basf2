#ifndef EUDAQ_INCLUDED_RawDataEvent
#define EUDAQ_INCLUDED_RawDataEvent

#include <sstream>

#include <vector>
#include <eutel/eudaq/Event.h>

namespace eudaq {

  /** An Event type consisting of just a vector of bytes.
   *
   */
  class RawDataEvent : public Event {
    EUDAQ_DECLARE_EVENT(RawDataEvent);
  public:
    typedef unsigned char byte_t;
    typedef std::vector<byte_t> data_t;
    struct block_t : public Serializable {
      block_t(unsigned id = (unsigned) - 1, data_t data = data_t()) : id(id), data(data) {}
      block_t(Deserializer&);
      void Serialize(Serializer&) const;
      void Append(const data_t& data);
      unsigned id;
      data_t data;
    };

    RawDataEvent(std::string type, unsigned run, unsigned event);
    RawDataEvent(Deserializer&);

    /// Add an empty block
    size_t AddBlock(unsigned id)
    {
      m_blocks.push_back(block_t(id));
      return m_blocks.size() - 1;
    }

    /// Add a data block as std::vector
    template <typename T>
    size_t AddBlock(unsigned id, const std::vector<T>& data)
    {
      m_blocks.push_back(block_t(id, make_vector(data)));
      return m_blocks.size() - 1;
    }

    /// Add a data block as array with given size
    template <typename T>
    size_t AddBlock(unsigned id, const T* data, size_t bytes)
    {
      m_blocks.push_back(block_t(id, make_vector(data, bytes)));
      return m_blocks.size() - 1;
    }

    /// Append data to a block as std::vector
    template <typename T>
    void AppendBlock(size_t index, const std::vector<T>& data)
    {
      m_blocks[index].Append(make_vector(data));
    }

    /// Append data to a block as array with given size
    template <typename T>
    void AppendBlock(size_t index, const T* data, size_t bytes)
    {
      m_blocks[index].Append(make_vector(data, bytes));
    }

    unsigned GetID(size_t i) const;
    /** Get the data block number i as vector of \c{unsigned char}, which is the byte sequence which
     *  which has been serialised. This is the recommended way to retrieve your
     *  data from the RawDataEvent since the other GetBlock functions might
     *  give different results depending on the endiannes of your mashine.
     */
    const data_t& GetBlock(size_t i) const;
    byte_t GetByte(size_t block, size_t index) const;

    /// Return the number of data blocks in the RawDataEvent
    size_t NumBlocks() const { return m_blocks.size(); }

    virtual void Print(std::ostream&) const;
    static RawDataEvent BORE(std::string type, unsigned run)
    {
      return RawDataEvent(type, run, (unsigned) - 1, Event::FLAG_BORE);
    }
    static RawDataEvent EORE(std::string type, unsigned run, unsigned event)
    {
      return RawDataEvent(type, run, event, Event::FLAG_EORE);
    }
    virtual void Serialize(Serializer&) const;

    /// Return the type string.
    virtual std::string GetSubType() const { return m_type; }

  private:
    // private constructor to create BORE and EORE
    // make sure that event number is 0 for BORE
    RawDataEvent(std::string type, unsigned run, unsigned event, Event::Flags flag)
      : Event(run, event, NOTIMESTAMP, flag) ,  m_type(type)
    {}

    template <typename T>
    static data_t make_vector(const T* data, size_t bytes)
    {
      const unsigned char* ptr = reinterpret_cast<const byte_t*>(data);
      return data_t(ptr, ptr + bytes);
    }

    template <typename T>
    static data_t make_vector(const std::vector<T>& data)
    {
      const unsigned char* ptr = reinterpret_cast<const byte_t*>(&data[0]);
      return data_t(ptr, ptr + data.size() * sizeof(T));
    }

    std::string m_type;
    std::vector<block_t> m_blocks;
  };

}

#endif // EUDAQ_INCLUDED_RawDataEvent
