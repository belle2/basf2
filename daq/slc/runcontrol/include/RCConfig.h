/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef Belle2_RCConfig_h
#define Belle2_RCConfig_h

#include <daq/slc/database/DBObject.h>

namespace Belle2 {

  class DBInterface;

  class RCConfig {

  public:
    RCConfig();
    RCConfig(const RCConfig& config);
    ~RCConfig() {}

  public:
    void print();
    DBObject& getObject() { return m_obj; }
    const DBObject& getObject() const { return m_obj; }
    void setObject(const DBObject& obj) { m_obj = obj; }
    int getExpNumber() const { return m_expno; }
    int getRunNumber() const { return m_runno; }
    int getSubNumber() const { return m_subno; }
    void setExpNumber(int expno) { m_expno = expno; }
    void setRunNumber(int runno) { m_runno = runno; }
    void setSubNumber(int subno) { m_subno = subno; }

  public:
    int getConfigId() const { return m_obj.getId(); }
    const std::string& getConfigName() const
    {
      return m_obj.getName();
    }

  private:
    DBObject m_obj;
    int m_expno;
    int m_runno;
    int m_subno;
    DBInterface* m_db;

  };

}

#endif
