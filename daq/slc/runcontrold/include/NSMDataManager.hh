#ifndef _B2DAQ_NSMDataManager_hh
#define _B2DAQ_NSMDataManager_hh

#include <node/NodeSystem.hh>
#include <nsm/RunStatus.hh>
#include <nsm/RunConfig.hh>

#include <map>
#include <vector>

namespace B2DAQ {

  class NSMDataManager {

  public:
    NSMDataManager(NodeSystem* system) throw();
    ~NSMDataManager() throw();

  public:
    RunStatus* allocateRunStatus() throw();
    RunConfig* allocateRunConfig() throw();
    RunStatus* getRunStatus() throw() { return _status; }
    RunConfig* getRunConfig() throw() { return _config; }
    bool writeRunStatus() throw();
    bool writeRunConfig() throw();
    void readNodeStatus() throw();
    std::vector<NSMData*>& getNodeStatus() { return _node_status_v; }

  private:
    NodeSystem* _system;
    RunStatus* _status;
    RunConfig* _config;
    std::vector<NSMData*> _node_status_v;

  };

}

#endif
