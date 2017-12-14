#include <eutel/eudaq/LogMessage.h>
#include <eutel/eudaq/Utils.h>

#include <ostream>
#include <istream>

namespace eudaq {

  namespace {

    inline std::string escape_char(char c)
    {
      if (c == '\n') return "\\n";
      if (c == '\t') return "\\t";
      return std::string(1, c);
    }

    static std::string escape_string(const std::string& s)
    {
      std::string result;
      result.reserve(s.length());
      for (std::string::const_iterator it = s.begin(); it != s.end(); ++it) {
        result += escape_char(*it);
      }
      return result;
    }

  }

  LogMessage::LogMessage(const std::string& msg, Level level, const Time& time)
    : Status(level, msg), m_line(0), m_time(time), m_createtime(Time::Current())
  {}

  LogMessage::LogMessage(Deserializer& ds)
    : Status(ds), m_time(0, 0), m_createtime(Time::Current())
  {
    ds.read(m_file);
    ds.read(m_func);
    ds.read(m_line);
    ds.read(m_time);
  }

  LogMessage LogMessage::Read(std::istream& is)
  {
//     os << Level2String(m_level) << "\t"
//        << escape_string(m_msg) << "\t"
//        << m_time.Formatted() << "\t"
//        << GetSender() << "\t"
//        << m_file << ":" << m_line << "\t"
//        << m_func << "\n";
    std::string line;
    std::getline(is, line);
    std::vector<std::string> parts = eudaq::split(line);
    if (parts.size() == 1 && line.substr(0, 3) == "***") EUDAQ_THROWX(FileFormatException, "Ignored line");
    if (parts.size() != 6) EUDAQ_THROWX(FileFormatException, "Badly formatted line");
    std::vector<std::string> timeparts = split(parts[2], " -.:");
    if (timeparts.size() != 7) EUDAQ_THROW("Badly formatted time: " + parts[2]);
    int level = 0;
    while (eudaq::Status::Level2String(level) != "" &&
           eudaq::Status::Level2String(level) != parts[0]) {
      level++;
    }
    eudaq::Time time(from_string(timeparts[0], 1970), from_string(timeparts[1], 1), from_string(timeparts[2], 1),
                     from_string(timeparts[3], 0), from_string(timeparts[4], 0), from_string(timeparts[5], 0),
                     from_string(timeparts[6], 0) * 1000);
    size_t colon = parts[4].find_last_of(":");
    return eudaq::LogMessage(parts[1], (eudaq::Status::Level)level, time)
           .SetLocation(parts[4].substr(0, colon),
                        from_string(parts[4].substr(colon + 1), 0),
                        parts[5])
           .SetSender(parts[3]);
  }

  void LogMessage::Serialize(Serializer& ser) const
  {
    Status::Serialize(ser);
    ser.write(m_file);
    ser.write(m_func);
    ser.write(m_line);
    ser.write(m_time);
  }

  void LogMessage::Print(std::ostream& os) const
  {
    os << Level2String(m_level) << ": " << m_msg
       << " " << m_time.Formatted();
    if (m_sendertype != "") os << " " << GetSender();
    if ((m_level <= LVL_DEBUG || m_level >= LVL_ERROR) && m_file != "") {
      os << " [in " << m_file << ":" << m_line;
      if (m_func != "") os << ", " << m_func;
      os << "]";
    }
  }

  void LogMessage::Write(std::ostream& os) const
  {
    os << Level2String(m_level) << "\t"
       << escape_string(m_msg) << "\t"
       << m_time.Formatted() << "\t"
       << GetSender() << "\t"
       << m_file << ":" << m_line << "\t"
       << m_func << "\n";
  }

  LogMessage& LogMessage::SetLocation(const std::string& file, unsigned line, const std::string& func)
  {
    m_file = file;
    m_line = line;
    m_func = func;
    return *this;
  }

  LogMessage& LogMessage::SetSender(const std::string& name)
  {
    size_t i = name.find('.');
    if (i == std::string::npos) {
      m_sendertype = name;
      m_sendername = "";
    } else {
      m_sendertype = name.substr(0, i);
      m_sendername = name.substr(i + 1);
    }
    return *this;
  }

}
