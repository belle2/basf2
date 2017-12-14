#include <eutel/eudaq/BufferSerializer.h>

namespace eudaq {

  BufferSerializer::BufferSerializer(Deserializer& des) : m_offset(0)
  {
    des.read(m_data);
  }

  void BufferSerializer::Serialize(Serializer& ser) const
  {
    ser.write(m_data);
  }

  void BufferSerializer::Serialize(const unsigned char* data, size_t len)
  {
    m_data.insert(m_data.end(), data, data + len);
  }

  void BufferSerializer::Deserialize(unsigned char* data, size_t len)
  {
    if (!len) return;
    if (len + m_offset > m_data.size()) {
      EUDAQ_THROW("Deserialize asked for " + to_string(len) +
                  ", only have " + to_string(m_data.size() - m_offset));
    }
    std::copy(&m_data[m_offset], &m_data[m_offset] + len, data);
    //m_data.erase(m_data.begin(), m_data.begin() + len);
    m_offset += len;
    //std::string tmp(data, data+len);
    //std::cout << "Deserialize: " << len << /*" \"" << tmp << "\"" <<*/ std::endl;
    //tmp = std::string(begin(), end());
    //std::cout << "Remaining: " << (end()-begin()) << /*" \"" << tmp << "\"" <<*/ std::endl;
  }

}
