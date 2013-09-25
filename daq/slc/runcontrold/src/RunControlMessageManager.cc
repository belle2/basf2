#include "RunControlMessageManager.hh"

#include "MessageBox.hh"
#include "DBRunInfoHandler.hh"

#include <db/DBNodeSystemConfigurator.hh>

#include <system/Time.hh>
#include <system/Date.hh>

#include <util/Debugger.hh>

using namespace B2DAQ;

void RunControlMessageManager::run()
{
  _ntry_recover = 0;
  {
    std::vector<NSMNode*>& node_v(_node_system->getNodes());
    for (size_t i = 0; i < node_v.size(); i++) {
      NSMNode* node(node_v[i]);
      _node_used_v.push_back(node->isUsed());
    }
  }
  //_node_seq_i = -1;
  //_cmd_seq = RCCommand::UNKNOWN;
  //_state_seq = RCState::UNKNOWN;
  while (true) {
    RunControlMessage msg = MessageBox::get().pop();
    RCCommand cmd = msg.getCommand();
    const NSMMessage& nsm(msg.getMessage());
    if (msg.getId() == RunControlMessage::GUI) {
      if (!cmd.isAvailable(_rc_node->getState())) continue;
      if (cmd == RCCommand::SET) {
        if (nsm.getParam(0) == RunControlMessage::FLAG_MODE) {
          _node_system->setOperationMode(nsm.getParam(1));
        } else if (nsm.getParam(0) == RunControlMessage::FLAG_OPERATORS) {
          _data_man->getRunConfig()->setOperators(nsm.getData());
        } else if (nsm.getParam(0) == RunControlMessage::FLAG_RUN_TYPE) {
          _data_man->getRunConfig()->setRunType(nsm.getData());
        } else if (nsm.getParam(0) == RunControlMessage::FLAG_RUN_VERSION) {
          _data_man->getRunConfig()->setVersion(nsm.getParam(1));
        } else if (nsm.getParam(0) == RunControlMessage::FLAG_RUN_LENGTH) {
        } else if (nsm.getParam(0) == RunControlMessage::FLAG_EXP_NO) {
          //_data_man->getRunStatus()->setExpNumber(nsm.getParam(1));
        } else {
          B2DAQ::debug("unknown command = %d %d", nsm.getParam(0), nsm.getParam(1));
        }
      } else {
        //_node_seq_i = 0;
        if (cmd == RCCommand::BOOT || cmd == RCCommand::REBOOT) {
          downloadConfig(cmd, _data_man->getRunConfig()->getVersion());
          send(cmd, RCState::BOOTING_TS);
        } else if (cmd == RCCommand::LOAD || cmd == RCCommand::RELOAD) {
          downloadConfig(cmd, _data_man->getRunConfig()->getVersion());
          send(cmd, RCState::LOADING_TS);
        } else if (cmd == RCCommand::START) {
          uploadRunConfig();
          send(cmd, RCState::STARTING_TS);
        } else if (cmd == RCCommand::STOP) {
          uploadRunResult();
          send(cmd, RCState::STOPPING_TS);
        } else if (cmd == RCCommand::PAUSE) {
          send(cmd, RCState::PAUSED_S);
        } else if (cmd == RCCommand::RESUME) {
          send(cmd, RCState::RUNNING_S);
        } else if (cmd == RCCommand::RECOVER) {
          send(cmd, RCState::RECOVERING_RS);
        } else if (cmd == RCCommand::ABORT) {
          send(cmd, RCState::ABORTING_RS);
        } else if (cmd == RCCommand::STATECHECK) {
          send(cmd, RCState::UNKNOWN);
        }
      }
    } else if (msg.getId() == RunControlMessage::LOCALNSM) {
      if (cmd == RCCommand::STATECHECK) {
        send(cmd, RCState::UNKNOWN);
      } else {
        int id = _comm->getMessage().getNodeId();
        NSMNode* node = findNode(id);
        if (node != NULL) {
          if (cmd == RCCommand::OK) {
            node->setState(RCState(nsm.getData()));
            handleOk(node);
          } else if (cmd == RCCommand::ERROR) {
            _rc_node->setState(RCState::ERROR_ES);
            node->setState(RCState::ERROR_ES);
            reportState(node, nsm.getData().c_str());
          }
        }
      }
    }
  }
}

NSMNode* RunControlMessageManager::findNode(int id) throw()
{
  NSMNode* node = getNodeByID(id);
  if (node == NULL) {
    std::string nodename = _comm->getMessage().getNodeName();
    node = getNodeByName(nodename);
    if (node == NULL) {
      B2DAQ::debug("Unexcepted node id: ", id);
      return NULL;
    }
  }
  return node;
}

NSMNode* RunControlMessageManager::getNodeByID(int id) throw()
{
  return (_node_id_m.find(id) != _node_id_m.end()) ? _node_id_m[id] : NULL;
}

NSMNode* RunControlMessageManager::getNodeByName(const std::string& name) throw()
{
  return (_node_name_m.find(name) != _node_name_m.end()) ? _node_name_m[name] : NULL;
}

void RunControlMessageManager::addNode(int id, NSMNode* node) throw()
{
  if (id >= 0 && node != NULL) {
    node->setNodeID(id);
    _node_id_m.insert(std::map<int, NSMNode*>::value_type(id, node));
    _node_name_m.insert(std::map<std::string, NSMNode*>::value_type(node->getName(), node));
  }
}

bool RunControlMessageManager::send(const RCCommand& command,
                                    const State& state_success) throw()
{
  RCState state_org = _rc_node->getState();
  bool is_success = true;
  std::vector<NSMNode*>& node_v(_node_system->getNodes());
  /*
  if (_node_seq_i < 0) return true;
  if (_node_seq_i == 0) {
    _cmd_seq = command;
    _state_seq = state_success;
  }
  if (_node_seq_i >= (int)node_v.size()) {
    _cmd_seq = RCCommand::UNKNOWN;
    _state_seq = RCState::UNKNOWN;
    _node_seq_i = -1;
    return true;
  }
  */
  try {
    for (size_t index = 0; index < node_v.size(); index++) {
      //NSMNode* node(node_v[_node_seq_i]);
      NSMNode* node(node_v[index]);
      //_node_seq_i++;
      if (!node->isUsed()) continue;
      int id = node->getNodeID();
      if (id < 0) id = _comm->getNodeIdByName(node->getName());
      int pid = (id >= 0) ? _comm->getNodePidByName(node->getName()) : -1;
      if (id >= 0 && pid > 0) {
        if (command != RCCommand::RECOVER) {
          _comm->sendRequest(node, command);
        } else {
          tryRecover(node);
        }
        node->setConnection(Connection::ONLINE);
        if (getNodeByID(id) == NULL) addNode(id, node);
        if (state_success != State::UNKNOWN) {
          node->setState(state_success);
          reportState(node);
        }
      } else {
        node->setConnection(Connection::OFFLINE);
        node->setState(State::UNKNOWN);
        reportState(node);
        _rc_node->setState(RCState::ERROR_ES);
        is_success = false;
      }
    }
  } catch (const IOException& e) {
    is_success = false;
  }
  _rc_node->setConnection(Connection::ONLINE);
  if (state_success != State::UNKNOWN && is_success) {
    _rc_node->setState(state_success);
  }
  reportState(state_org);
  return is_success;
}

void RunControlMessageManager::handleOk(NSMNode* node)
{
  RCState state_org = _rc_node->getState();
  RCState next;
  bool synchronized = false;
  std::vector<NSMNode*>& node_v(_node_system->getNodes());
  if (state_org.isTransaction()) {
    next = state_org.next();
    for (size_t i = 0; i < node_v.size(); i++) {
      if (!node_v[i]->isUsed()) continue;
      synchronized = (next == node_v[i]->getState());
      if (!synchronized) break;
    }
  } else if (state_org == RCState::RECOVERING_RS) {
    synchronized = tryRecover(node);
    next = RCState::READY_S;
    for (size_t i = 0; i < node_v.size(); i++) {
      if (!node_v[i]->isUsed()) continue;
      if (!synchronized) break;
      synchronized = (next == node_v[i]->getState());
    }
  } else {
    next = node->getState();
    synchronized = false;
    for (size_t i = 0; i < node_v.size(); i++) {
      if (!node_v[i]->isUsed()) continue;
      synchronized = (next == node_v[i]->getState());
      if (!synchronized) break;
    }
  }
  reportState(node);
  if (synchronized && state_org != next) {
    _rc_node->setState(next);
    reportState(state_org);
  }
}

bool RunControlMessageManager::reportState(NSMNode* node, const std::string& data) throw()
{
  NSMMessage nsm;
  nsm.setNParams(3);
  nsm.setParam(0, node->getIndex());
  nsm.setParam(1, node->getConnection().getId());
  nsm.setParam(2, node->getState().getId());
  if (data.size() > 0) nsm.setData(data);
  RunControlMessage msg(RunControlMessage::LOCALNSM, nsm);
  msg.setCommand(RCCommand::STATE);
  _ui_comm->sendMessage(msg);
  return true;
}

bool RunControlMessageManager::reportState(const State& state_org) throw()
{
  if ((state_org == RCState::STARTING_TS &&
       _rc_node->getState() == RCState::RUNNING_S) ||
      (state_org == RCState::RUNNING_S &&
       _rc_node->getState() == RCState::STOPPING_TS)) {
    NSMMessage nsm;
    nsm.setNParams(7);
    nsm.setParam(0, -1);
    nsm.setParam(1, _rc_node->getConnection().getId());
    nsm.setParam(2, _rc_node->getState().getId());
    RunControlMessage msg(RunControlMessage::RUNCONTROLLER, nsm);
    msg.setCommand(RCCommand::STATE);
    _ui_comm->sendMessage(msg);
  } else {
    return reportState(_rc_node);
  }
  return true;
}

bool RunControlMessageManager::tryRecover(NSMNode* node)
{
  if (node->getState() == RCState::INITIAL_S) {
    _comm->sendRequest(node, RCCommand::BOOT);
  } else if (node->getState() == RCState::CONFIGURED_S) {
    _comm->sendRequest(node, RCCommand::LOAD);
  } else if (node->getState() == RCState::READY_S) {
    _ntry_recover = 0;
    return true;
  } else if (node->getState() == RCState::PAUSED_S) {
    _comm->sendRequest(node, RCCommand::STOP);
  } else if (node->getState() == RCState::RUNNING_S) {
    _comm->sendRequest(node, RCCommand::STOP);
  } else if (node->getState() == RCState::ERROR_ES) {
    if (_ntry_recover < 4) {
      _comm->sendRequest(node, RCCommand::RECOVER);
      _ntry_recover++;
    } else {
      node->setState(RCState::FATAL_ES);
    }
  }
  return false;
}

void RunControlMessageManager::downloadConfig(const RCCommand& cmd, int version) throw()
{
  DBNodeSystemConfigurator config(_db, _node_system);
  try {
    config.readTables(version);
  } catch (const IOException& e) {
    B2DAQ::debug("Error on loading system configuration.:%s", e.what());
  }
  _data_man->writeData();
  std::vector<NSMNode*>& node_v(_node_system->getNodes());
  for (size_t i = 0; i < node_v.size(); i++) {
    NSMNode* node(node_v[i]);
    bool used = _node_used_v[i];
    _node_used_v[i] = node->isUsed();
    if (node->isUsed() && !used) {
      _comm->sendRequest(node, RCCommand::BOOT);
      if (cmd == RCCommand::RELOAD) {
        _comm->sendRequest(node, RCCommand::LOAD);
      }
    }
  }
}

void RunControlMessageManager::uploadRunConfig() throw()
{
  RunStatus* status = _data_man->getRunStatus();
  RunConfig* config = _data_man->getRunConfig();
  status->incrementRunNumber();
  status->setStartTime(Time().getSecond());
  try {
    _data_man->writeRunStatus();
    _data_man->writeRunConfig();
  } catch (const IOException& e) {
    B2DAQ::debug("Error on writing run configuration to NSM.:%s", e.what());
  }
  DBRunInfoHandler handler(_db, status, config);
  try {
    handler.createRunConfigTable();
  } catch (const IOException& e) {}
  try {
    handler.writeRunConfigTable();
  } catch (const IOException& e) {
    B2DAQ::debug("Error on uploading run configuration.:%s", e.what());
  }
  NSMMessage nsm;
  nsm.setNParams(2);
  nsm.setParam(0, RunControlMessage::FLAG_EXP_NO);
  nsm.setParam(1, status->getExpNumber());
  RunControlMessage msg(RunControlMessage::RUNCONTROLLER, nsm);
  msg.setCommand(RCCommand::SET);
  _ui_comm->sendMessage(msg);

  nsm.setParam(0, RunControlMessage::FLAG_RUN_NO);
  nsm.setParam(1, status->getRunNumber());
  msg.setMessage(nsm);
  _ui_comm->sendMessage(msg);

  nsm.setParam(0, RunControlMessage::FLAG_START_TIME);
  nsm.setParam(1, status->getStartTime());
  msg.setMessage(nsm);
  _ui_comm->sendMessage(msg);
}

void RunControlMessageManager::uploadRunResult() throw()
{
  RunStatus* status = _data_man->getRunStatus();
  RunConfig* config = _data_man->getRunConfig();
  status->setEndTime(Time().getSecond());
  try {
    _data_man->writeRunStatus();
    _data_man->writeRunConfig();
  } catch (const IOException& e) {
    B2DAQ::debug("Error on writing run configuration to NSM.:%s", e.what());
  }
  DBRunInfoHandler handler(_db, status, config);
  try {
    handler.createRunStatusTable();
  } catch (const IOException& e) {}
  try {
    handler.writeRunStatusTable();
  } catch (const IOException& e) {
    B2DAQ::debug("Error on uploading run status.:%s", e.what());
  }
  NSMMessage nsm;
  nsm.setNParams(2);
  nsm.setParam(0, RunControlMessage::FLAG_END_TIME);
  nsm.setParam(1, status->getEndTime());
  RunControlMessage msg(RunControlMessage::RUNCONTROLLER, nsm);
  msg.setCommand(RCCommand::SET);
  _ui_comm->sendMessage(msg);
}
