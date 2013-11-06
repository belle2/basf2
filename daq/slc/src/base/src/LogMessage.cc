#include "base/LogMessage.h"

#include "base/StringUtil.h"

#include <sstream>
#include <cstdlib>

using namespace Belle2;

std::string LogMessage::__hostname = "";

LogMessage::LogMessage(const std::string& node_name, Priority priority,
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

const std::string LogMessage::toString() const
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
  }
  ss << _message;
  return ss.str();
}

bool LogMessage::unpack(int npar, const int* pars,
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

int LogMessage::pack(int* pars, std::string& str_out) const
{
  pars[0] = (int)_priority;
  pars[1] = (int)_log_date.get();
  pars[2] = (int)_ref_no;
  str_out = _group_name + ";" + _hostname + ";" + _node_name + ";" + _message;
  return 3;
}
