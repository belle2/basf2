#ifndef _Belle2_DAQLogMessage_h
#define _Belle2_DAQLogMessage_h

#include "daq/slc/base/Date.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/database/ConfigObject.h"

#include <string>

namespace Belle2 {

  class DAQLogMessage : public ConfigObject {

  private:
    static const std::string g_tablename;
    static const int g_revision;

  public:
    DAQLogMessage() throw();
    DAQLogMessage(const std::string& node_name,
                  LogFile::Priority priority,
                  const std::string& message) throw();
    DAQLogMessage(const DAQLogMessage& log) throw();
    virtual ~DAQLogMessage() throw() {}

  public:
    void setPriority(LogFile::Priority priority) throw();
    void setNodeName(const std::string& name) throw();
    void setMessage(const std::string& message) throw();
    void setDate() throw();
    void setDate(int date) throw();
    void setDate(const Date& date) throw();
    LogFile::Priority getPriority() const throw();
    const std::string getNodeName() const throw();
    const std::string getMessage() const throw();
    int getDateInt() const throw();
    const Date getDate() const throw();

  };

}

#endif
