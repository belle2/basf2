#ifndef _Belle2_RCCallback_hh
#define _Belle2_RCCallback_hh

#include "daq/slc/runcontrol/RCState.h"
#include "daq/slc/runcontrol/RCCommand.h"
#include "daq/slc/runcontrol/RCConfig.h"
#include "daq/slc/runcontrol/RCHandlerException.h"

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>
#include <daq/slc/system/PThread.h>

namespace Belle2 {

  class DBInterface;

  class RCCallback : public NSMCallback {

    friend class RCMonitor;

  public:
    RCCallback(int timeout = 4) throw();
    virtual ~RCCallback() throw() {}

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void load(const DBObject&) throw(RCHandlerException) {}
    virtual void start(int /*expno*/, int /*runno*/) throw(RCHandlerException) {}
    virtual void stop() throw(RCHandlerException) {}
    virtual void recover() throw(RCHandlerException) {}
    virtual void resume() throw(RCHandlerException) {}
    virtual void pause() throw(RCHandlerException) {}
    virtual void abort() throw(RCHandlerException) {}

  public:
    virtual bool perform(NSMCommunicator& com) throw();

  public:
    virtual bool initialize(const DBObject&) throw() { return true; }
    virtual bool configure(const DBObject&) throw() { return true; }

  public:
    void setState(const RCState& state) throw();
    void setRuntype(const std::string& runtype) { m_runtype = runtype; }
    void setDBTable(const std::string& table) { m_table = table; }
    const std::string& getDBTable() const { return m_table; }
    void setAutoReply(bool auto_reply) { m_auto = auto_reply; }
    void setDB(DBInterface* db, const std::string& table);
    DBInterface* getDB() { return m_db; }
    void setProvider(const std::string& host, int port) {
      m_provider_host = host;
      m_provider_port = port;
    }

  private:
    void dbload(NSMCommunicator& com) throw(IOException);

  private:
    RCState m_state_demand;
    DBObject m_obj;
    DBInterface* m_db;
    std::string m_table;
    bool m_auto;
    std::string m_runtype;
    std::string m_provider_host;
    int m_provider_port;

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
