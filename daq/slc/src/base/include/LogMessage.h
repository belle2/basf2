#ifndef _BELLE2_LOGMESSAGE_H_
#define _BELLE2_LOGMESSAGE_H_

#include "base/Date.h"

#include <string>

namespace Belle2 {

  class LogMessage {

  public:
    LogMessage() {}
    LogMessage(const std::string& process_name,
               int priority, const std::string& log_date,
               const std::string& message);
    LogMessage(const std::string& hostname,
               const std::string& process_name,
               int priority, const std::string& log_date,
               const std::string& message);
    LogMessage(const std::string& process_name,
               int priority, const std::string& message);
    LogMessage(const std::string& hostname,
               const std::string& process_name,
               int priority, const std::string& message);
    ~LogMessage() {}

  public:
    const std::string encode();
    bool decode(const std::string& str);

  private:
    std::string _hostname;
    std::string _process_name;
    int _priority;
    std::string _log_date;
    std::string _message;

  };

}

#endif

