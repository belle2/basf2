#ifndef _Belle2_NodeSystem_hh
#define _Belle2_NodeSystem_hh

#include "base/NSMNode.h"
#include "base/NodeGroup.h"
#include "base/RunStatus.h"
#include "base/RunConfig.h"

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class NodeSystem {

  public:
    static const int MODE_GLOBAL;
    static const int MODE_LOCAL;

  public:
    NodeSystem(int revision = 0)
      : _revision(revision), _rc_node(NULL) {}
    virtual ~NodeSystem() throw();

  public:
    const std::string& getName() const { return _name; }
    int getRevision() const { return _revision; }
    NSMNode* getRunControlNode() { return _rc_node; }
    void setRunControlNode(NSMNode* node) { _rc_node = node; }
    std::vector<NSMNode*>& getNodes() { return _node_v; }
    std::vector<NodeGroup*>& getNodeGroups() { return _group_v; }
    RunStatus* getStatus() { return _status; }
    RunConfig* getConfig() { return _config; }
    void addNode(NSMNode* node) { _node_v.push_back(node); }
    void addNodeGroup(NodeGroup* group);
    void setName(const std::string& name) { _name = name; }
    void setRevision(int revision) { _revision = revision; }
    void setStatus(RunStatus* status) { _status = status; }
    void setConfig(RunConfig* config) { _config = config; }

  private:
    std::string _name;
    int _revision;
    RunStatus* _status;
    RunConfig* _config;
    NSMNode* _rc_node;
    std::vector<NSMNode*> _node_v;
    std::vector<NodeGroup*> _group_v;

  };

}

#endif
