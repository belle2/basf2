#ifndef _BELLE2_LOGMESSAGE_H_
#define _BELLE2_LOGMESSAGE_H_

#include <base/Date.h>

#include <string>

namespace Belle2 {

  class LogMessage {

  public:
    enum Priority {
      DEBUG, INFO, NOTICE, WARNING, ERROR, FATAL
    };

  public:
    static std::string __hostname;

  public:
    LogMessage(const std::string& proces_name = "",
               Priority priority = DEBUG,
               const std::string& message = "");
    ~LogMessage() {}

  public:
    const std::string toString() const;
    bool unpack(int npar, const int* pars,
                const std::string& str_in);
    int pack(int* pars, std::string& str_out) const;
    void setProcessName(const std::string& name) {
      _process_name = name;
    }

  private:
    Priority _priority;
    Date _log_date;
    std::string _hostname;
    std::string _process_name;
    std::string _message;

  };

}

#endif

