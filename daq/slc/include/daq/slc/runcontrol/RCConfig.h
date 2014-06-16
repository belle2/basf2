#ifndef Belle2_RCConfig_h
#define Belle2_RCConfig_h

#include <daq/slc/database/ConfigObject.h>

namespace Belle2 {

  class DBInterface;

  class RCConfig {

  public:
    RCConfig();
    RCConfig(const RCConfig& config);
    ~RCConfig() throw() {}

  public:
    void print() throw();
    ConfigObject& getObject() throw() { return m_obj; }
    const ConfigObject& getObject() const throw() { return m_obj; }
    void setObject(const ConfigObject& obj) throw() { m_obj = obj; }
    int getExpNumber() const throw() { return m_expno; }
    int getRunNumber() const throw() { return m_runno; }
    int getSubNumber() const throw() { return m_subno; }
    void setExpNumber(int expno) throw() { m_expno = expno; }
    void setRunNumber(int runno) throw() { m_runno = runno; }
    void setSubNumber(int subno) throw() { m_subno = subno; }

  public:
    int getConfigId() const throw() { return m_obj.getId(); }
    const std::string& getConfigName() const throw() {
      return m_obj.getName();
    }
    const std::string& getNodeName() const throw() {
      return m_obj.getNode();
    }

  private:
    ConfigObject m_obj;
    int m_expno;
    int m_runno;
    int m_subno;
    DBInterface* m_db;

  };

}

#endif
