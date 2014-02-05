#ifndef _Belle2_EnvMonitorMaster_h
#define _Belle2_EnvMonitorMaster_h

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/Cond.h>

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/nsm/NSMData.h>

namespace Belle2 {

  class EnvMonitorMaster {

  public:
    EnvMonitorMaster(DBInterface* db, NSMCommunicator* comm)
      : _db(db), _comm(comm) {}
    ~EnvMonitorMaster() throw() {}

  public:
    void add(NSMData* data) { _data_v.push_back(data); }
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }
    void wait() { _cond.wait(_mutex); }
    void notify() { _cond.broadcast(); }
    std::vector<NSMData*>& getData() { return _data_v; }

  public:
    void run();

  private:
    DBInterface* _db;
    NSMCommunicator* _comm;
    std::vector<NSMData*> _data_v;
    Cond _cond;
    Mutex _mutex;

  };

}

#endif
