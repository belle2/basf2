#include "daq/slc/base/SystemLog.h"

#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/Reader.h"
#include "daq/slc/base/Writer.h"

#include <sstream>
#include <cstdlib>

using namespace Belle2;

std::string SystemLog::g_hostname = "";

SystemLog::SystemLog(const std::string& node_name, Priority priority,
                     const std::string& message, int ref_no)
  : m_priority(priority), m_log_date(), m_hostname(),
    m_node_name(node_name), m_message(message), m_ref_no(ref_no)
{
  if (g_hostname.size() == 0) {
    const char* hostname = getenv("HOSTNAME");
    if (hostname != NULL) {
      g_hostname = hostname;
    }
  }
  if (g_hostname.size() > 0) {
    m_hostname = g_hostname;
  }
}

const std::string SystemLog::toString() const
{
  std::stringstream ss;
  ss << "[from=" << m_hostname << "/" << m_node_name
     << "] [date=" << m_log_date.toString();
  switch (m_priority) {
    case DEBUG:   ss << "] [DEBUG] "; break;
    case INFO:    ss << "] [INFO] "; break;
    case NOTICE:  ss << "] [NOTICE] "; break;
    case WARNING: ss << "] [WARNING] "; break;
    case ERROR:   ss << "] [ERROR] "; break;
    case FATAL:   ss << "] [FATAL] "; break;
    default:      ss << "] [UNKNOWN] "; break;
  }
  ss << m_message;
  return ss.str();
}

void SystemLog::readObject(Reader& reader) throw(IOException)
{
  m_priority = (Priority)reader.readInt();
  m_log_date = Date((time_t)reader.readInt());
  m_ref_no = reader.readInt();
  m_group_name = reader.readString();
  m_hostname = reader.readString();
  m_node_name = reader.readString();
  m_message = reader.readString();
}

void SystemLog::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeInt((int)m_priority);
  writer.writeInt((int)m_log_date.get());
  writer.writeInt(m_ref_no);
  writer.writeString(m_group_name);
  writer.writeString(m_hostname);
  writer.writeString(m_node_name);
  writer.writeString(m_message);
}

