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
    LogMessage(const std::string& node_name = "",
               Priority priority = DEBUG,
               const std::string& message = "",
               int ref_no = -1);
    ~LogMessage() {}

  public:
    const std::string toString() const;
    bool unpack(int npar, const int* pars,
                const std::string& str_in);
    int pack(int* pars, std::string& str_out) const;
    void setNodeName(const std::string& name) {
      _node_name = name;
    }
    void setGroupName(const std::string& name) {
      _group_name = name;
    }

  private:
    Priority _priority;
    Date _log_date;
    std::string _group_name;
    std::string _hostname;
    std::string _node_name;
    std::string _message;
    int _ref_no;

  };

}

#endif

