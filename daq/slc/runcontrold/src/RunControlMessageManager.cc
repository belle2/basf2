#include "RunControlMessageManager.hh"

#include "MessageBox.hh"
#include "DBRunInfoHandler.hh"

#include <db/DBNodeSystemConfigurator.hh>

#include <system/Time.hh>
#include <system/Date.hh>

#include <util/Debugger.hh>

#include <unistd.h>

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
  int index_seq = -1;
  Command cmd_seq;
  while (true) {
    RunControlMessage msg = MessageBox::get().pop();
    usleep(100);
    Command cmd = msg.getCommand();
    const NSMMessage& nsm(msg.getMessage());
    if (msg.getId() == RunControlMessage::GUI) {
      if (nsm.getParam(0) > 0) {
        NSMNode* node = _node_system->getNodes()[nsm.getParam(0)];
        State state_next = getNextState(cmd);
        if (state_next != State::UNKNOWN)
          node->setState(state_next);
        reportState(node);
        _comm->sendRequest(node, cmd);
      } else {
        if (!cmd.isAvailable(_rc_node->getState())) continue;
        index_seq = 0;
        cmd_seq = cmd;
        State state_next = getNextState(cmd);
        if (state_next != State::UNKNOWN)
          _rc_node->setState(state_next);
        if (cmd == Command::BOOT) {
          _data_man->getRunConfig()->setVersion(nsm.getParam(1));
          downloadConfig(cmd, _data_man->getRunConfig()->getVersion());
        } else if (cmd == Command::LOAD) {
          _data_man->getRunConfig()->setVersion(nsm.getParam(1));
          _data_man->getRunConfig()->setOperators(nsm.getData());
          downloadConfig(cmd, _data_man->getRunConfig()->getVersion());
        } else if (cmd == Command::START) {
          uploadRunConfig();
        } else if (cmd == Command::STOP) {
          uploadRunResult();
        } else if (cmd == Command::STATECHECK) {
          reportRCStatus();
        }
        reportRCStatus();
        std::vector<NSMNode*>& node_v(_node_system->getNodes());
        for (size_t i = 0; i < node_v.size(); i++) {
          State state_org = node_v[i]->getState();
          node_v[i]->setState(_rc_node->getState());
          reportState(node_v[i]);
        }
        index_seq = distribute(index_seq, cmd_seq);
      }
    } else if (msg.getId() == RunControlMessage::LOCALNSM) {
      if (index_seq < 0 && cmd == Command::STATECHECK) {
        std::vector<NSMNode*>& node_v(_node_system->getNodes());
        for (size_t i = 0; i < node_v.size(); i++) {
          if (node_v[i]->isUsed())
            send(node_v[i], cmd);
        }
      } else {
        int id = nsm.getNodeId();
        NSMNode* node = findNode(id);
        if (node != NULL) {
          if (cmd == Command::OK) {
            node->setState(State(nsm.getData()));
            reportState(node);
            std::vector<NSMNode*>& node_v(_node_system->getNodes());
            for (size_t i = 0; i < node_v.size(); i++) {
              if (node_v[i]->isUsed())
                reportState(node_v[i]);
            }
            if (isSynchronized(node->getState(), node_v.size())) {
              _rc_node->setState(node->getState());
              reportState(_rc_node);
            }
            if (index_seq >= (int)node_v.size()) index_seq = -1;
            if (index_seq > 0) {
              State next_state = State(_rc_node->getState()).next();
              if (!node_v[index_seq]->isSynchronize() ||
                  isSynchronized(next_state, index_seq)) {
                index_seq = distribute(index_seq, cmd_seq);
              }
            }
          } else if (cmd == Command::ERROR) {
            reportError(node, nsm.getData());
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
      B2DAQ::debug("Unexcepted node id: %d", id);
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

bool RunControlMessageManager::recover(NSMNode* node)
{
  if (!node->isUsed()) return true;
  if (node->getState() == State::INITIAL_S) {
    _comm->sendRequest(node, Command::BOOT);
  } else if (node->getState() == State::CONFIGURED_S) {
    _comm->sendRequest(node, Command::LOAD);
  } else if (node->getState() == State::READY_S) {
    return true;
  } else if (node->getState() == State::PAUSED_S) {
    _comm->sendRequest(node, Command::STOP);
  } else if (node->getState() == State::RUNNING_S) {
    _comm->sendRequest(node, Command::STOP);
  } else if (node->getState() == State::ERROR_ES) {
    _comm->sendRequest(node, Command::ABORT);
  }
  return false;
}

State RunControlMessageManager::getNextState(const Command& cmd)
{
  if (cmd == Command::BOOT) {
    return State::BOOTING_TS;
  } else if (cmd == Command::LOAD) {
    return State::LOADING_TS;
  } else if (cmd == Command::START) {
    return State::STARTING_TS;
  } else if (cmd == Command::STOP) {
    return State::STOPPING_TS;
  } else if (cmd == Command::PAUSE) {
    return State::PAUSED_S;
  } else if (cmd == Command::RESUME) {
    return State::RUNNING_S;
  } else if (cmd == Command::ABORT) {
    return State::ABORTING_RS;
  }
  return State::UNKNOWN;
}

bool RunControlMessageManager::isSynchronized(const State& state, int size)
{
  std::vector<NSMNode*>& node_v(_node_system->getNodes());
  for (int i = 0; i < size; i++) {
    if (node_v[i]->isUsed() &&
        node_v[i]->getState() != state) return false;
  }
  return true;
}

int RunControlMessageManager::distribute(int index_seq, const Command& command) throw()
{
  std::vector<NSMNode*>& node_v(_node_system->getNodes());
  for (int i = index_seq; i < (int)node_v.size(); i++) {
    if (node_v[i]->isUsed()) {
      if (i > index_seq && node_v[i]->isSynchronize()) return i;
      if (!send(node_v[i], command)) {
        return -1;
      }
    }
  }
  return (int)node_v.size();
}

bool RunControlMessageManager::send(NSMNode* node, const Command& command) throw()
{
  try {
    int id = node->getNodeID();
    if (id < 0) id = _comm->getNodeIdByName(node->getName());
    int pid = (id >= 0) ? _comm->getNodePidByName(node->getName()) : -1;
    if (id >= 0 && pid > 0) {
      int pars[256];
      std::string data = "";
      int npar = node->getParams(command, pars, data);
      _comm->sendRequest(node, command, npar, pars, data);
      if (getNodeByID(id) == NULL) addNode(id, node);
      node->setConnection(Connection::ONLINE);
      return true;
    } else {
      node->setConnection(Connection::OFFLINE);
      node->setState(State::UNKNOWN);
      reportState(node);
    }
  } catch (const IOException& e) {
    B2DAQ::debug("%s:%d : %s", __FILE__, __LINE__, e.what());
  }
  if (_rc_node->getState() != State::ERROR_ES) {
    _rc_node->setState(State::ERROR_ES);
    reportRCStatus();
  }
  return false;
}

bool RunControlMessageManager::reportState(NSMNode* node, const std::string& data) throw()
{
  NSMMessage nsm;
  nsm.setNParams(3);
  nsm.setParam(0, node->getIndex());
  nsm.setParam(1, node->getConnection().getId());
  nsm.setParam(2, node->getState().getId());
  if (data.size() > 0) nsm.setData(data);
  RunControlMessage msg(RunControlMessage::RUNCONTROLLER, nsm);
  msg.setCommand(Command::OK);
  try {
    _ui_comm->sendMessage(msg);
  } catch (const IOException& e) {
    B2DAQ::debug("%s:%d error=%s", __FILE__, __LINE__, e.what());
  }
  if (node != _rc_node && node->getState() == State::ERROR_ES) {
    _rc_node->setState(State::ERROR_ES);
    reportState(_rc_node);
  }
  return true;
}

bool RunControlMessageManager::reportError(NSMNode* node, const std::string& data) throw()
{
  NSMMessage nsm;
  nsm.setNParams(1);
  nsm.setParam(0, node->getIndex());
  if (data.size() > 0) nsm.setData(data);
  RunControlMessage msg(RunControlMessage::RUNCONTROLLER, nsm);
  msg.setCommand(Command::ERROR);
  try {
    _ui_comm->sendMessage(msg);
  } catch (const IOException& e) {
    B2DAQ::debug("%s:%d error=%s", __FILE__, __LINE__, e.what());
  }
  return true;
}

bool RunControlMessageManager::reportRCStatus() throw()
{
  RunStatus* status = _data_man->getRunStatus();
  RunConfig* config = _data_man->getRunConfig();
  NSMMessage nsm;
  nsm.setNParams(8);
  nsm.setParam(0, -1);
  nsm.setParam(1, _rc_node->getConnection().getId());
  nsm.setParam(2, _rc_node->getState().getId());
  nsm.setParam(3, config->getVersion());
  nsm.setParam(4, status->getExpNumber());
  nsm.setParam(5, status->getRunNumber());
  nsm.setParam(6, status->getStartTime());
  nsm.setParam(7, status->getEndTime());
  nsm.setData(config->getRunType() + "\n" + config->getOperators());
  RunControlMessage msg(RunControlMessage::RUNCONTROLLER, nsm);
  msg.setCommand(Command::OK);
  try {
    _ui_comm->sendMessage(msg);
  } catch (const IOException& e) {
    B2DAQ::debug("%s:%d error=%s", __FILE__, __LINE__, e.what());
  }
  return true;
}

void RunControlMessageManager::downloadConfig(const Command& cmd, int version) throw()
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
      _comm->sendRequest(node, Command::BOOT);
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
}
