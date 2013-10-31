#ifndef _Belle2_NSMDataManager_hh
#define _Belle2_NSMDataManager_hh

#include <base/NodeSystem.h>
#include <nsm/RunStatus.h>
#include <nsm/RunConfig.h>

#include <map>
#include <vector>

namespace Belle2 {

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
