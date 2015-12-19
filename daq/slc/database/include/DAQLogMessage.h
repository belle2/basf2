#ifndef _Belle2_DAQLogMessage_h
#define _Belle2_DAQLogMessage_h

#include "daq/slc/base/Date.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/database/DBObject.h"

#include <string>

namespace Belle2 {

  class DAQLogMessage {

  public:
    DAQLogMessage() throw();
    DAQLogMessage(const std::string& nodename,
                  LogFile::Priority priority,
                  const std::string& message) throw();
    DAQLogMessage(const std::string& nodename,
                  LogFile::Priority priority,
                  const std::string& message,
                  const Date& date) throw();
    DAQLogMessage(const std::string& nodename,
                  const std::string& priority,
                  const std::string& message,
                  const Date& date) throw();
    DAQLogMessage(const DAQLogMessage& log) throw();
    virtual ~DAQLogMessage() throw() {}

  public:
    void setId(int id) throw() { m_id = id; }
    void setPriority(const std::string& priority) throw();
    void setPriority(LogFile::Priority priority) throw();
    void setNodeName(const std::string& name) throw();
    void setMessage(const std::string& message) throw();
    void setDate() throw();
    void setDate(int date) throw();
    void setDate(const Date& date) throw();
    int getId() const throw() { return m_id; }
    LogFile::Priority getPriority() const throw();
    int getPriorityInt() const throw();
    const std::string getPriorityText() const throw();
    const std::string& getNodeName() const throw();
    const std::string& getMessage() const throw();
    int getDateInt() const throw();
    const Date getDate() const throw();

  private:
    int m_date;
    std::string m_nodename;
    int m_priority;
    std::string m_message;
    int m_id;

  };

}

#endif
