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
    static std::string g_hostname;

  public:
    SystemLog(const std::string& node_name = "", Priority priority = DEBUG,
              const std::string& message = "", int ref_no = -1);
    virtual ~SystemLog() throw() {}

  public:
    const std::string toString() const;
    void setPriority(Priority priority) { m_priority = priority; }
    void setNodeName(const std::string& name) { m_node_name = name; }
    void setGroupName(const std::string& name) { m_group_name = name; }
    void setHostName(const std::string& name) { m_hostname = name; }
    void setMessage(const std::string& message) { m_message = message; }
    void setDate() { m_log_date = Date(); }
    void setDate(int date) { m_log_date = Date(date); }
    void setDate(const Date& date) { m_log_date = date; }
    Priority getPriority() const { return m_priority; }
    const std::string& getNodeName() const { return m_node_name; }
    const std::string& getGroupName() const { return m_group_name; }
    const std::string& getHostName() const { return m_hostname; }
    const std::string& getMessage() const { return m_message; }
    const Date& getDate() const { return m_log_date; }
    virtual void readObject(Reader& reader) throw(IOException);
    virtual void writeObject(Writer& writer) const throw(IOException);

  private:
    Priority m_priority;
    Date m_log_date;
    std::string m_group_name;
    std::string m_hostname;
    std::string m_node_name;
    std::string m_message;
    int m_ref_no;

  };

}

#endif

