#include <ostream>
#include <iostream>

#include <eutel/eudaq/Event.h>

namespace eudaq {

  namespace {

    static const char* const FLAGNAMES[] = {
      "BORE",
      "EORE",
      "HITS",
      "FAKE",
      "SIMU"
    };

  }

  Event::Event(Deserializer& ds)
  {
    ds.read(m_flags);
    ds.read(m_runnumber);
    ds.read(m_eventnumber);
    ds.read(m_timestamp);
    ds.read(m_tags);
  }

  void Event::Serialize(Serializer& ser) const
  {
    //std::cout << "Serialize id = " << std::hex << get_id() << std::endl;
    ser.write(get_id());
    ser.write(m_flags);
    ser.write(m_runnumber);
    ser.write(m_eventnumber);
    ser.write(m_timestamp);
    ser.write(m_tags);
  }

  void Event::Print(std::ostream& os) const
  {
    os << "Type=" << id2str(get_id()) << ":" << GetSubType()
       << ", Number=" << m_runnumber << "." << m_eventnumber;
    if (m_timestamp != NOTIMESTAMP)
      os << ", Time=0x" << to_hex(m_timestamp, 16);
    if (m_flags) {
      unsigned f = m_flags;
      bool first = true;
      for (size_t i = 0; f > 0; ++i, f >>= 1) {
        if (f & 1) {
          os << (first ? ", Flags=" : ",")
             << (i < sizeof FLAGNAMES / sizeof * FLAGNAMES ? std::string(FLAGNAMES[i]) : to_string(i));
          first = false;
        }
      }
    }
    if (m_tags.size() > 0) {
      for (map_t::const_iterator i = m_tags.begin(); i != m_tags.end(); ++i) {
        os << (i == m_tags.begin() ? ", {" : ", ")  << i->first << "=" << i->second;
      }
      os << "}";
    }
  }

  unsigned Event::str2id(const std::string& str)
  {
    unsigned long result = 0;
    for (size_t i = 0; i < 4; ++i) {
      if (i < str.length()) result |= str[i] << (8 * i);
    }
    //std::cout << "str2id(" << str << ") = " << std::hex << result << std::dec << std::endl;
    return result;
  }

  std::string Event::id2str(unsigned id)
  {
    //std::cout << "id2str(" << std::hex << id << std::dec << ")" << std::flush;
    std::string result(4, '\0');
    for (int i = 0; i < 4; ++i) {
      result[i] = (char)(id & 0xff);
      id >>= 8;
    }
    for (int i = 3; i >= 0; --i) {
      if (result[i] == '\0') {
        result.erase(i);
        break;
      }
    }
    //std::cout << " = " << result << std::endl;
    return result;
  }

  Event& Event::SetTag(const std::string& name, const std::string& val)
  {
    m_tags[name] = val;
    return *this;
  }

  std::string Event::GetTag(const std::string& name, const std::string& def) const
  {
    std::map<std::string, std::string>::const_iterator i = m_tags.find(name);
    if (i == m_tags.end()) return def;
    return i->second;
  }

  std::ostream& operator << (std::ostream& os, const Event& ev)
  {
    ev.Print(os);
    return os;
  }

  EventFactory::map_t& EventFactory::get_map()
  {
    static map_t s_map;
    return s_map;
  }

  void EventFactory::Register(unsigned long id, EventFactory::event_creator func)
  {
    // TODO: check id is not already in map
    get_map()[id] = func;
  }

  EventFactory::event_creator EventFactory::GetCreator(unsigned long id)
  {
    // TODO: check it exists...
    return get_map()[id];
  }

}
