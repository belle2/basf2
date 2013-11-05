#ifndef _Belle2_RunControlMessageManager_hh
#define _Belle2_RunControlMessageManager_hh

#include "LocalNSMCommunicator.h"
#include "GUICommunicator.h"

#include "xml/NodeLoader.h"

#include "database/DBInterface.h"
#include "database/DBNodeSystemConfigurator.h"

#include "nsm/NSMCommunicator.h"

#include "base/NSMNode.h"
#include "base/NodeSystem.h"
#include "base/Command.h"
#include "base/State.h"

#include <map>

namespace Belle2 {

  class RunControlMessageManager {

  public:
    RunControlMessageManager(DBInterface* db,
                             LocalNSMCommunicator* nsm_comm,
                             GUICommunicator* ui_comm,
                             NSMNode* rc_node, NodeLoader* loader)
      : _db(db), _nsm_comm(nsm_comm), _ui_comm(ui_comm),
        _rc_node(rc_node), _node_system(&(loader->getSystem())),
        _dbconfig(db, loader) {}
    ~RunControlMessageManager();

  public:
    void run();

  private:
    bool recover(NSMNode* node);
    bool isSynchronized(const State& state, int size);
    int distribute(int index, const Command& command) throw();
    bool send(NSMNode* node, const Command& command,
              int npar = 0, const unsigned int* pars = NULL) throw();
    void downloadConfig(const Command& cmd, int version) throw();
    void uploadRunConfig(const Command& cmd,
                         const std::string& data) throw();
    void uploadRunResult(const Command& cmd) throw();
    bool reportError(NSMNode* node, const std::string& message = "") throw();
    bool reportState(NSMNode* node, const std::string& message = "") throw();
    bool reportRCStatus() throw();
    State getNextState(const Command& cmd);
    void updateRunStatus(const State& state_org);
    NSMNode* findNode(int id,  const NSMMessage& msg) throw();
    NSMNode* getNodeByID(int id) throw();
    NSMNode* getNodeByName(const std::string& name) throw();
    void addNode(int id, NSMNode* node) throw();

  private:
    DBInterface* _db;
    LocalNSMCommunicator* _nsm_comm;
    GUICommunicator* _ui_comm;
    NSMNode* _rc_node;
    NodeSystem* _node_system;
    DBNodeSystemConfigurator _dbconfig;
    std::map<int, NSMNode*> _node_id_m;
    std::map<std::string, NSMNode*> _node_name_m;
    std::vector<bool> _node_used_v;
    int _ntry_recover;
    int _node_seq_i;
    Command _cmd_seq;
    State _state_seq;

  };

}

#endif

