#ifndef _Belle2_HVControlMaster_h
#define _Belle2_HVControlMaster_h

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/Cond.h>

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMData.h>

#include "daq/slc/apps/hvcontrold/HVNodeInfo.h"

#include <map>

namespace Belle2 {

  typedef std::map<std::string, NSMData*> HVNSMDataList;
  typedef std::map<std::string, HVNodeInfo> HVNodeInfoList;
  typedef std::vector<std::string> HVNodeNameList;

  class HVControlMaster {

  public:
    HVControlMaster() {}
    virtual ~HVControlMaster() throw() {}

  public:
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }
    void wait() { _cond.wait(_mutex); }
    void notify() { _cond.signal(); }
    void addNode(NSMNode* node, const std::string& filename);
    bool hasNode(const std::string& id) throw() { return _node_m.find(id) != _node_m.end(); }
    HVNodeInfo& getNodeInfo(const std::string& id) { return _node_m[id]; }
    NSMData* getData(const std::string& id) { return _nsmdata_m[id]; }
    HVNodeNameList& getNodeNameList() { return _name_v; }
    HVNodeInfoList& getNodeInfoList() { return _node_m; }
    HVNSMDataList& getDataList() { return _nsmdata_m; }
    void setDB(DBInterface* db) { _db = db; }
    DBInterface* getDB() { return _db; }
    void createTables();

  private:
    Mutex _mutex;
    Cond _cond;
    HVNodeNameList _name_v;
    HVNodeInfoList _node_m;
    HVNSMDataList _nsmdata_m;
    DBInterface* _db;

  };

}

#endif
