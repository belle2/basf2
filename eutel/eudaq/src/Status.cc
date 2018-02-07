#include <eutel/eudaq/Status.h>
#include <eutel/eudaq/Exception.h>
#include <eutel/eudaq/Utils.h>

namespace eudaq {

  Status::Status(Deserializer& ds)
  {
    ds.read(m_level);
    ds.read(m_msg);
    ds.read(m_tags);
  }

  void Status::Serialize(Serializer& ser) const
  {
    ser.write(m_level);
    ser.write(m_msg);
    ser.write(m_tags);
  }

  std::string Status::Level2String(int level)
  {
    static const char* const strings[] = {
      "DEBUG",
      "OK",
      "THROW",
      "EXTRA",
      "INFO",
      "WARN",
      "ERROR",
      "USER",
      "BUSY",
      "NONE"
    };
    if (level < LVL_DEBUG || level > LVL_NONE) return "";
    return strings[level];
  }

  int Status::String2Level(const std::string& str)
  {
    int lvl = 0;
    std::string tmpstr1, tmpstr2 = ucase(str);
    while ((tmpstr1 = Level2String(lvl)) != "") {
      if (tmpstr1 == tmpstr2) return lvl;
      lvl++;
    }
    EUDAQ_THROW("Unrecognised level: " + str);
  }

  Status& Status::SetTag(const std::string& name, const std::string& val)
  {
    m_tags[name] = val;
    return *this;
  }

  std::string Status::GetTag(const std::string& name, const std::string& def) const
  {
    std::map<std::string, std::string>::const_iterator i = m_tags.find(name);
    if (i == m_tags.end()) return def;
    return i->second;
  }

  void Status::print(std::ostream& os) const
  {
    os << Level2String(m_level);
    if (m_msg != "") {
      os << ": " << m_msg;
    }
  }

}
