#include "base/LogMessage.h"

#include "base/StringUtil.h"

#include <sstream>
#include <cstdlib>

using namespace Belle2;

LogMessage::LogMessage(const std::string& process_name,
                       int priority, const std::string& log_date,
                       const std::string& message)
  : _hostname(), _process_name(process_name),
    _priority(priority), _log_date(log_date), _message(message)
{
  const char* hostname = getenv("HOSTNAME");
  if (hostname != NULL) {
    _hostname = hostname;
  }
}

LogMessage::LogMessage(const std::string& hostname,
                       const std::string& process_name,
                       int priority, const std::string& log_date,
                       const std::string& message)
  : _hostname(hostname), _process_name(process_name),
    _priority(priority), _log_date(log_date), _message(message)
{
}

LogMessage::LogMessage(const std::string& process_name,
                       int priority, const std::string& message)
  : _hostname(), _process_name(process_name),
    _priority(priority), _log_date(Date().toString()), _message(message)
{
  const char* hostname = getenv("HOSTNAME");
  if (hostname != NULL) {
    _hostname = hostname;
  }
}

LogMessage::LogMessage(const std::string& hostname,
                       const std::string& process_name,
                       int priority, const std::string& message)
  : _hostname(hostname), _process_name(process_name),
    _priority(priority), _log_date(Date().toString()), _message(message)
{
}

const std::string LogMessage::encode()
{
  std::stringstream ss;
  ss << _hostname << ";"
     << _process_name << ";"
     << _priority << ";"
     << _log_date << ";"
     << _message;
  return ss.str();
}

bool LogMessage::decode(const std::string& str_in)
{
  std::string str = Belle2::replace(Belle2::replace(str_in, "\n", ""), "\r", "");
  std::vector<std::string> str_v = Belle2::split(str, ';');
  if (str_v.size() < 5) return false;
  std::string priority_s = Belle2::replace(str_v[2], " ", "");
  if (!isdigit(priority_s.at(0))) return false;
  _hostname = Belle2::replace(str_v[0], " ", "");
  _process_name = Belle2::replace(str_v[1], " ", "");
  _priority = atoi(priority_s.c_str());
  _log_date = str_v[3];
  _message = str_v[4];
  return true;
}
