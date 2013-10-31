#ifndef _Belle2_NodeSystem_hh
#define _Belle2_NodeSystem_hh

#include "NSMNode.h"
#include "NodeGroup.h"
#include "RONode.h"
#include "TTDNode.h"
#include "COPPERNode.h"
#include "HSLB.h"
#include "FEEModule.h"
#include "Host.h"

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class NodeSystem {

  public:
    static const int MODE_GLOBAL;
    static const int MODE_LOCAL;

  public:
    NodeSystem(int version = 0)
      : _version(version), _operation_mode(MODE_LOCAL), _rc_node(NULL),
        _run_type("TEST:DEFAULT"), _description("default setups") {}
    virtual ~NodeSystem() throw();

  public:
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();

  public:
    NSMNode* getRunControlNode() { return _rc_node; }
    void setRunControlNode(NSMNode* node) { _rc_node = node; }
    void setVersion(int version) { _version = version; }
    void setVersion(const std::string& label, int version);
    void setOperationMode(int operation_mode) { _operation_mode = operation_mode; }
    void setRunType(const std::string& run_type) { _run_type = run_type; }
    void setDescription(const std::string& description) { _description = description; }
    int getVersion() const { return _version; }
    int getOperationMode() const { return _operation_mode; }
    const std::string& getRunType() const { return _run_type; }
    const std::string& getDescription() const { return _description; }
    std::vector<Host*>& getHosts() { return _host_v; }
    std::vector<NSMNode*>& getNodes() { return _node_v; }
    std::vector<NodeGroup*>& getNodeGroups() { return _group_v; }
    std::vector<RONode*>& getRONodes() { return _ro_v; }
    std::vector<TTDNode*>& getTTDNodes() { return _ttd_v; }
    std::vector<FTSW*>& getFTSWs() { return _ftsw_v; }
    std::vector<COPPERNode*>& getCOPPERNodes() { return _copper_v; }
    std::vector<HSLB*>& getHSLBs() { return _hslb_v; }
    std::vector<FEEModule*>& getModules() { return _module_v; }
    std::vector<FEEModule*>& getModules(const std::string& module_class) {
      return _module_v_m[module_class];
    }
    std::map<std::string, std::vector<FEEModule*> >& getModuleLists() {
      return _module_v_m;
    }
    void addHost(Host* host) { _host_v.push_back(host); }
    void addNode(NSMNode* node) { _node_v.push_back(node); }
    void addNodeGroup(NodeGroup* group);
    void addRONode(RONode* node) { _ro_v.push_back(node); }
    void addCOPPERNode(COPPERNode* node) { _copper_v.push_back(node); }
    void addTTDNode(TTDNode* node) { _ttd_v.push_back(node); }
    void addFTSW(FTSW* ftsw) { _ftsw_v.push_back(ftsw); }
    void addHSLB(HSLB* hslb) { _hslb_v.push_back(hslb); }
    void addModule(FEEModule* module) { _module_v.push_back(module); }
    void addModules(const std::string module_class, std::vector<FEEModule*> module_v) {
      _module_v_m.insert(std::map<std::string, std::vector<FEEModule*> >::value_type(module_class, module_v));
    }
    bool hasModuleClass(const std::string& module_class) throw() {
      return _module_v_m.find(module_class) != _module_v_m.end();
    }

  private:
    int _version;
    int _operation_mode;
    NSMNode* _rc_node;
    std::string _run_type;
    std::string _description;
    std::string _date;
    std::vector<Host*> _host_v;
    std::vector<NSMNode*> _node_v;
    std::vector<NodeGroup*> _group_v;
    std::vector<TTDNode*> _ttd_v;
    std::vector<FTSW*> _ftsw_v;
    std::vector<RONode*> _ro_v;
    std::vector<COPPERNode*> _copper_v;
    std::vector<HSLB*> _hslb_v;
    std::vector<FEEModule*> _module_v;
    std::vector<std::string> _copper_name_v;
    mutable std::map<std::string, std::vector<FEEModule*> > _module_v_m;

  };

}

#endif
