#include "daq/slc/base/SystemLog.h"

#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/Reader.h"
#include "daq/slc/base/Writer.h"

#include <sstream>
#include <cstdlib>

using namespace Belle2;

std::string SystemLog::__hostname = "";

SystemLog::SystemLog(const std::string& node_name, Priority priority,
                     const std::string& message, int ref_no)
  : _priority(priority), _log_date(), _hostname(),
    _node_name(node_name), _message(message), _ref_no(ref_no)
{
  if (__hostname.size() == 0) {
    const char* hostname = getenv("HOSTNAME");
    if (hostname != NULL) {
      __hostname = hostname;
    }
  }
  if (__hostname.size() > 0) {
    _hostname = __hostname;
  }
}

const std::string SystemLog::toString() const
{
  std::stringstream ss;
  ss << "[from=" << _hostname << "/" << _node_name
     << "] [date=" << _log_date.toString();
  switch (_priority) {
    case DEBUG:   ss << "] [DEBUG] "; break;
    case INFO:    ss << "] [INFO] "; break;
    case NOTICE:  ss << "] [NOTICE] "; break;
    case WARNING: ss << "] [WARNING] "; break;
    case ERROR:   ss << "] [ERROR] "; break;
    case FATAL:   ss << "] [FATAL] "; break;
    default:      ss << "] [UNKNOWN] "; break;
  }
  ss << _message;
  return ss.str();
}

void SystemLog::readObject(Reader& reader) throw(IOException)
{
  _priority = (Priority)reader.readInt();
  _log_date = Date((time_t)reader.readInt());
  _ref_no = reader.readInt();
  _group_name = reader.readString();
  _hostname = reader.readString();
  _node_name = reader.readString();
  _message = reader.readString();
}

void SystemLog::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeInt((int)_priority);
  writer.writeInt((int)_log_date.get());
  writer.writeInt(_ref_no);
  writer.writeString(_group_name);
  writer.writeString(_hostname);
  writer.writeString(_node_name);
  writer.writeString(_message);
}

