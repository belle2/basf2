#ifndef _Belle2_EnvMonitorMaster_h
#define _Belle2_EnvMonitorMaster_h

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/Cond.h>

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/nsm/NSMData.h>

namespace Belle2 {

  class EnvMonitorMaster {

  public:
    EnvMonitorMaster(DBInterface& db, NSMCommunicator& comm)
      : m_db(db), m_comm(comm) {}
    ~EnvMonitorMaster() throw() {}

  public:
    void add(const NSMData& data) { m_data_v.push_back(data); }
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }
    void wait() { m_cond.wait(m_mutex); }
    void notify() { m_cond.broadcast(); }
    std::vector<NSMData>& getData() { return m_data_v; }

  public:
    void run();

  private:
    DBInterface& m_db;
    NSMCommunicator& m_comm;
    std::vector<NSMData> m_data_v;
    Cond m_cond;
    Mutex m_mutex;

  };

}

#endif
