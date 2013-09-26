#ifndef _B2DAQ_RunControlMessageManager_hh
#define _B2DAQ_RunControlMessageManager_hh

#include "HostCommunicator.hh"
#include "NSMDataManager.hh"

#include <runcontrol/RCCommand.hh>
#include <runcontrol/RCState.hh>

#include <db/DBInterface.hh>

#include <nsm/NSMCommunicator.hh>
#include <nsm/RunStatus.hh>
#include <nsm/RunConfig.hh>

#include <node/NSMNode.hh>
#include <node/NodeSystem.hh>

#include <map>

namespace B2DAQ {

  class RunControlMessageManager {

  public:
    RunControlMessageManager(DBInterface* db, NSMCommunicator* comm, 
			     NSMDataManager* data_man, HostCommunicator* ui_comm,
			     NSMNode* rc_node, NodeSystem* node_system)
      : _db(db), _comm(comm), _data_man(data_man), 
	_ui_comm(ui_comm), _rc_node(rc_node),
	_node_system(node_system) {}
    ~RunControlMessageManager();

  public:
    void run();

  private:
    bool recover(NSMNode* node);
    bool isSynchronized(const RCState& state, int size);
    int distribute(int index, const RCCommand& command) throw();
    bool send(NSMNode* node, const RCCommand& command) throw();
    void downloadConfig(const RCCommand& command, int version) throw();
    void uploadRunConfig() throw();
    void uploadRunResult() throw();
    bool reportState(NSMNode* node, const std::string& message = "") throw();
    RCState getNextState(const RCCommand& cmd);
    void updateRunStatus(const State& state_org);
    NSMNode* findNode(int id) throw();
    NSMNode* getNodeByID(int id) throw();
    NSMNode* getNodeByName(const std::string& name) throw();
    void addNode(int id, NSMNode* node) throw();
  
  private:
    DBInterface* _db;
    NSMCommunicator* _comm;
    NSMDataManager* _data_man;
    HostCommunicator* _ui_comm;
    NSMNode* _rc_node;
    NodeSystem* _node_system;
    std::map<int, NSMNode*> _node_id_m;
    std::map<std::string, NSMNode*> _node_name_m;
    std::vector<bool> _node_used_v;
    int _ntry_recover;
    int _node_seq_i;
    RCCommand _cmd_seq;
    RCState _state_seq;

  };

}

#endif

