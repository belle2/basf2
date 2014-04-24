#ifndef _BELLE2_SYSTEMLOG_H_
#define _BELLE2_SYSTEMLOG_H_

#include "daq/slc/base/Date.h"
#include "daq/slc/base/Serializable.h"

#include <string>

namespace Belle2 {

  class SystemLog : public Serializable {

  public:
    enum Priority {
      UNKNOWN = 0, DEBUG, INFO, NOTICE, WARNING, ERROR, FATAL
    };

  public:
    static std::string __hostname;

  public:
    SystemLog(const std::string& node_name = "", Priority priority = DEBUG,
              const std::string& message = "", int ref_no = -1);
    virtual ~SystemLog() throw() {}

  public:
    const std::string toString() const;
    void setPriority(Priority priority) { _priority = priority; }
    void setNodeName(const std::string& name) { _node_name = name; }
    void setGroupName(const std::string& name) { _group_name = name; }
    void setHostName(const std::string& name) { _hostname = name; }
    void setMessage(const std::string& message) { _message = message; }
    void setDate() { _log_date = Date(); }
    void setDate(int date) { _log_date = Date(date); }
    void setDate(const Date& date) { _log_date = date; }
    Priority getPriority() const { return _priority; }
    const std::string& getNodeName() const { return _node_name; }
    const std::string& getGroupName() const { return _group_name; }
    const std::string& getHostName() const { return _hostname; }
    const std::string& getMessage() const { return _message; }
    const Date& getDate() const { return _log_date; }
    virtual void readObject(Reader& reader) throw(IOException);
    virtual void writeObject(Writer& writer) const throw(IOException);

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

