#ifndef _B2DAQ_RunControlCallback_hh
#define _B2DAQ_RunControlCallback_hh

#include <runcontrol/RCCallback.hh>
#include <runcontrol/RCCommand.hh>
#include <runcontrol/RCState.hh>

#include <nsm/RunStatus.hh>

#include <node/NSMNode.hh>
#include <node/NodeSystem.hh>

#include <map>

namespace B2DAQ {

  class RunControlCallback : public RCCallback {

  public:
    RunControlCallback(NSMNode* rc_node = NULL,
		       NodeSystem* node_system = NULL,
		       RunStatus* status = NULL,
		       NSMNode* master_node = NULL) 
      throw(): RCCallback(rc_node), _rc_node(rc_node), _node_system(node_system),
	       _status(status), _master_node(master_node) {}
    virtual ~RunControlCallback() throw() {}

  public:
    virtual bool boot() throw() { return send(RCCommand::BOOT, RCState::BOOTING_TS); }
    virtual bool reboot() throw() { return send(RCCommand::REBOOT, RCState::BOOTING_TS); }
    virtual bool load() throw() { return send(RCCommand::LOAD, RCState::LOADING_TS); }
    virtual bool reload() throw() { return send(RCCommand::RELOAD, RCState::LOADING_TS); }
    virtual bool start() throw() { return send(RCCommand::START, RCState::STARTING_TS); }
    virtual bool stop() throw() { return send(RCCommand::STOP, RCState::STOPPING_TS); }
    virtual bool resume() throw() { return send(RCCommand::RESUME, RCState::RUNNING_S); }
    virtual bool pause() throw() { return send(RCCommand::PAUSE, RCState::PAUSED_S); }
    virtual bool stateCheck() throw() { return send(RCCommand::STATECHECK, State::UNKNOWN); }
    virtual bool recover() throw() { return send(RCCommand::RECOVER, RCState::RECOVERING_RS); }
    virtual bool abort() throw() { return send(RCCommand::ABORT, RCState::ABORTING_RS); }
    virtual bool state() throw() { return ok(); }
    virtual bool ok() throw();
    virtual bool error() throw();
    virtual void selfCheck() throw(NSMHandlerException) {
      stateCheck();
    }

  private:
    bool send(const RCCommand& command, const State& state_success) throw();
    bool reportState(NSMNode* node) throw();
    bool reportState(const State& state_org) throw();
    bool tryRecover(NSMNode* node);
    void updateRunStatus(const State& state_org);
    NSMNode* getNodeByID(int id) throw() {
      return (_node_id_m.find(id) != _node_id_m.end())?_node_id_m[id]:NULL;
    }
    NSMNode* getNodeByName(const std::string& name) throw() {
      return (_node_name_m.find(name) != _node_name_m.end())?_node_name_m[name]:NULL;
    }
    void addNode(int id, NSMNode* node) throw() {
      if ( id >=0 && node != NULL ) {
	node->setNodeID(id);
	_node_id_m.insert(std::map<int, NSMNode*>::value_type(id, node));
	_node_name_m.insert(std::map<std::string, NSMNode*>::value_type(node->getName(), node));
      }
    }

  private:
    NSMNode* _rc_node;
    NodeSystem* _node_system;
    RunStatus* _status;
    NSMNode* _master_node;
    std::map<int, NSMNode*> _node_id_m;
    std::map<std::string, NSMNode*> _node_name_m;

  };

}

#endif
