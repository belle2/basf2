#ifndef _Belle2_RCCallback_hh
#define _Belle2_RCCallback_hh

#include "daq/slc/runcontrol/RCState.h"
#include "daq/slc/runcontrol/RCCommand.h"
#include "daq/slc/runcontrol/RCConfig.h"
#include "daq/slc/runcontrol/RCHandlerException.h"
#include "daq/slc/runcontrol/RCHandlerFatalException.h"

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>
#include <daq/slc/system/PThread.h>

namespace Belle2 {

  class DBInterface;

  class RCCallback : public NSMCallback {

    friend class RCMonitor;
    friend class RCConfigHandler;

  public:
    RCCallback(int timeout = 4) throw();
    virtual ~RCCallback() throw() {}

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void boot(const std::string& opt, const DBObject&) throw(RCHandlerException) {}
    virtual void load(const DBObject&) throw(RCHandlerException) {}
    virtual void start(int /*expno*/, int /*runno*/) throw(RCHandlerException) {}
    virtual void stop() throw(RCHandlerException) {}
    virtual void recover(const DBObject&) throw(RCHandlerException) {}
    virtual bool resume(int /*subno*/) throw(RCHandlerException) { return true; }
    virtual bool pause() throw(RCHandlerException) { return true; }
    virtual void abort() throw(RCHandlerException) {}
    virtual void monitor() throw(RCHandlerException) {}

  public:
    virtual bool perform(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  public:
    virtual void initialize(const DBObject&) throw(RCHandlerException) {}
    virtual void configure(const DBObject&) throw(RCHandlerException) {}

  public:
    void setState(const RCState& state) throw();
    void setRCConfig(const std::string& rcconfig) { m_rcconfig = rcconfig; }
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
    DBObject& getDBObject() throw() { return m_obj; }
    void setRunNumbers(int expno, int runno)
    {
      m_expno = expno;
      m_runno = runno;
    }
    int getExpNumber() const { return m_expno; }
    int getRunNumber() const { return m_runno; }

  private:
    void dbload(int length, const char* data) throw(IOException);

  public:
    DBObject dbload(const std::string& path);
  protected:
    void dbrecord(DBObject obj, int expno, int runno, bool isstart) throw(IOException);

  private:
    RCState m_state_demand;
    DBObject m_obj;
    DBInterface* m_db;
    std::string m_table;
    std::string m_file;
    bool m_auto;
    std::string m_rcconfig;
    std::string m_provider_host;
    int m_provider_port;
    int m_expno;
    int m_runno;

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
