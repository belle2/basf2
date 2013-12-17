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

bool SystemLog::unpack(int npar, const int* pars,
                       const std::string& str_in)
{
  if (npar < 3 || pars[0] < (int)DEBUG ||
      pars[0] > (int)FATAL || pars[1] <= 0) return false;
  std::vector<std::string> str_v = Belle2::split(str_in, ';');
  if (str_v.size() < 4) return false;
  _priority = (Priority)pars[0];
  _log_date = Date((time_t)pars[1]);
  _ref_no = pars[2];
  _group_name = Belle2::replace(str_v[0], " ", "");
  _hostname = Belle2::replace(str_v[1], " ", "");
  _node_name = Belle2::replace(str_v[2], " ", "");
  _message = str_v[3];
  for (size_t i = 4; i < str_v.size(); i++) {
    _message += ";" + str_v[i];
  }
  return true;
}

int SystemLog::pack(int* pars, std::string& str_out) const
{
  pars[0] = (int)_priority;
  pars[1] = (int)_log_date.get();
  pars[2] = (int)_ref_no;
  str_out = _group_name + ";" + _hostname + ";" + _node_name + ";" + _message;
  return 3;
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

const std::string SystemLog::toSQLConfig() const
{
  std::stringstream ss;
  ss << "id serial, record_time timestamp, priority smallint, "
     << "log_date int, groupname text, hostname text, node text, "
     << "refno smallint, message text";
  return ss.str();
}

const std::string SystemLog::toSQLNames() const
{
  std::stringstream ss;
  ss << "record_time, priority, log_date, groupname, "
     << "hostname, node, refno, message";
  return ss.str();
}

const std::string SystemLog::toSQLValues() const
{
  std::stringstream ss;
  ss << "current_timestamp," << _priority << ","
     << "" << _log_date.get() << ",'" << _group_name << "',"
     << "'" << _hostname << "', '" << _node_name << "',"
     << "" << _ref_no << ",'" << Belle2::replace(_message, "'", "") << "'";
  return ss.str();
}

