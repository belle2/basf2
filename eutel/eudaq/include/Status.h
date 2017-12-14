#ifndef EUDAQ_INCLUDED_Status
#define EUDAQ_INCLUDED_Status

#include <eutel/eudaq/Serializable.h>
#include <eutel/eudaq/Serializer.h>
#include <string>
#include <ostream>

namespace eudaq {

  class Status : public Serializable {
  public:
    enum Level {
      LVL_DEBUG,
      LVL_OK,
      LVL_THROW,
      LVL_EXTRA,
      LVL_INFO,
      LVL_WARN,
      LVL_ERROR,
      LVL_USER,
      LVL_BUSY,
      LVL_NONE  // The last value, any additions should go before this
    };
    Status(int level = LVL_OK, const std::string& msg = "")
      : m_level(level), m_msg(msg) {}
    Status(Deserializer&);
    virtual void Serialize(Serializer&) const;

    Status& SetTag(const std::string& name, const std::string& val);
    std::string GetTag(const std::string& name, const std::string& def = "") const;
    static std::string Level2String(int level);
    static int String2Level(const std::string&);
    virtual ~Status() {}
    virtual void print(std::ostream&) const;
    int GetLevel() const { return m_level; }
  protected:
    typedef std::map<std::string, std::string> map_t;
    int m_level;
    std::string m_msg;
    map_t m_tags; ///< Metadata tags in (name=value) pairs of strings
  };

  inline std::ostream& operator << (std::ostream& os, const Status& s)
  {
    s.print(os);
    return os;
  }


}

#endif // EUDAQ_INCLUDED_Status
