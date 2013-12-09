#ifndef _Belle2_EnvDBRecorder_h
#define _Belle2_EnvDBRecorder_h

#include <daq/slc/dqm/EnvMonitorPackage.h>

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/Mutex.h>

namespace Belle2 {

  class EnvDBRecorder {

  private:
    static Mutex __mutex;
    static DBInterface* __db;

  public:
    static void setDB(DBInterface* db) {
      __db = db;
    }

  public:
    EnvDBRecorder(EnvMonitorPackage* monitor)
      : _monitor(monitor) {}
    ~EnvDBRecorder() throw() {}

  public:
    void run();

  private:
    EnvMonitorPackage* _monitor;

  };

}

#endif
