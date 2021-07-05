/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef _Belle2_RCCallback_hh
#define _Belle2_RCCallback_hh

#include "daq/slc/runcontrol/RCState.h"
#include "daq/slc/runcontrol/RCConfig.h"

#include <daq/slc/nsm/NSMCallback.h>

namespace Belle2 {

  class DBInterface;

  class RCCallback : public NSMCallback {

    friend class RCMonitor;
    friend class RCConfigHandler;

  public:
    RCCallback(int timeout = 4);
    virtual ~RCCallback() {}

  public:
    virtual void init(NSMCommunicator& com);
    virtual void boot(const std::string& /* opt */, const DBObject&) {}
    virtual void load(const DBObject&, const std::string& /*runtype*/) {}
    virtual void start(int /*expno*/, int /*runno*/) {}
    virtual void stop() {}
    virtual void recover(const DBObject&, const std::string& /*runtype*/) {}
    virtual bool resume(int /*subno*/) { return true; }
    virtual bool pause() { return true; }
    virtual void abort() {}
    virtual void monitor() {}
    virtual std::string dbdump();

  public:
    virtual bool perform(NSMCommunicator& com);
    virtual void timeout(NSMCommunicator& com);

  public:
    virtual void initialize(const DBObject&) {}
    virtual void configure(const DBObject&) {}
    void dump(bool isstart);

  public:
    const NSMNode& getRuncontrol() const { return m_runcontrol; }
    void setState(const RCState& state);
    void setRCConfig(const std::string& rcconfig) { m_rcconfig_org = rcconfig; }
    void setDBTable(const std::string& table) { m_table = table; }
    void setDBFile(const std::string& file) { m_file = file; }
    const std::string& getDBTable() const { return m_table; }
    void setAutoReply(bool auto_reply) { m_auto = auto_reply; }
    void setDB(DBInterface* db, const std::string& table);
    DBInterface* getDB() { return m_db; }
    void setProvider(const std::string& host, int port)
    {
      m_provider_host = host;
      m_provider_port = port;
    }
    DBObject& getDBObject() { return m_obj; }
    void setRunNumbers(int expno, int runno)
    {
      m_expno = expno;
      m_runno = runno;
    }
    int getExpNumber() const { return m_expno; }
    int getRunNumber() const { return m_runno; }
    void setRunTypeRecord(const std::string& runtype) { m_runtype_record = runtype; }
    const std::string& getRunTypeRecord() const { return m_runtype_record; }

  private:
    void dbload(int length, const char* data);

  public:
    DBObject dbload(const std::string& path);
    void configure_raw(int length, const char* data);

  private:
    RCState m_state_demand;
    DBObject m_obj;
    DBInterface* m_db;
    std::string m_table;
    std::string m_file;
    bool m_auto;
    std::string m_rcconfig;
    std::string m_rcconfig_org;
    std::string m_provider_host;
    int m_provider_port;
    int m_expno;
    int m_runno;
    NSMNode m_runcontrol;
    std::string m_runtype_record;

  protected:
    bool m_showall;

  };

  inline void RCCallback::setDB(DBInterface* db, const std::string& table)
  {
    m_db = db;
    m_table = table;
  }

};

#endif
