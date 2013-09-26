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
  RCCommand cmd_seq;
  while (true) {
    RunControlMessage msg = MessageBox::get().pop();
    usleep(100);
    RCCommand cmd = msg.getCommand();
    std::cout << __FILE__ << ":" << __LINE__ << " " << cmd.getLabel() << std::endl;
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
        } else {
          B2DAQ::debug("unknown parameter = %d, value = %d", nsm.getParam(0), nsm.getParam(1));
        }
      } else {
        index_seq = 0;
        cmd_seq = cmd;
        if (cmd == RCCommand::BOOT || cmd == RCCommand::REBOOT) {
          downloadConfig(cmd, _data_man->getRunConfig()->getVersion());
          _rc_node->setState(RCState::BOOTING_TS);
        } else if (cmd == RCCommand::LOAD || cmd == RCCommand::RELOAD) {
          downloadConfig(cmd, _data_man->getRunConfig()->getVersion());
          _rc_node->setState(RCState::LOADING_TS);
        } else if (cmd == RCCommand::START) {
          uploadRunConfig();
          _rc_node->setState(RCState::STARTING_TS);
        } else if (cmd == RCCommand::STOP) {
          uploadRunResult();
          _rc_node->setState(RCState::STOPPING_TS);
        } else if (cmd == RCCommand::PAUSE) {
          _rc_node->setState(RCState::PAUSED_S);
        } else if (cmd == RCCommand::RESUME) {
          _rc_node->setState(RCState::RUNNING_S);
        } else if (cmd == RCCommand::RECOVER) {
          _rc_node->setState(RCState::RECOVERING_RS);
        } else if (cmd == RCCommand::ABORT) {
          _rc_node->setState(RCState::ABORTING_RS);
        }
        reportState(_rc_node);
        std::vector<NSMNode*>& node_v(_node_system->getNodes());
        for (size_t i = 0; i < node_v.size(); i++) {
          node_v[i]->setState(_rc_node->getState());
          reportState(node_v[i]);
        }
        index_seq = distribute(index_seq, cmd_seq) ;
      }
    } else if (msg.getId() == RunControlMessage::LOCALNSM) {
      if (index_seq < 0 && cmd == RCCommand::STATECHECK) {
        std::vector<NSMNode*>& node_v(_node_system->getNodes());
        for (size_t i = 0; i < node_v.size(); i++) {
          if (node_v[i]->isUsed())
            send(node_v[i], cmd);
        }
      } else {
        int id = _comm->getMessage().getNodeId();
        //std::cout << __FILE__ << ":" << __LINE__ << " " << id << std::endl;
        NSMNode* node = findNode(id);
        if (node != NULL) {
          if (cmd == RCCommand::OK) {
            node->setState(RCState(nsm.getData()));
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
              RCState next_state = RCState(_rc_node->getState()).next();
              if (!node_v[index_seq]->isSynchronize() ||
                  isSynchronized(next_state, index_seq)) {
                index_seq = distribute(index_seq, cmd_seq);
              }
            }
          } else if (cmd == RCCommand::ERROR) {
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

bool RunControlMessageManager::isSynchronized(const RCState& state, int size)
{
  std::vector<NSMNode*>& node_v(_node_system->getNodes());
  for (int i = 0; i < size; i++) {
    if (node_v[i]->isUsed() &&
        node_v[i]->getState() != state) return false;
  }
  return true;
}

int RunControlMessageManager::distribute(int index_seq, const RCCommand& command) throw()
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

bool RunControlMessageManager::send(NSMNode* node, const RCCommand& command) throw()
{
  try {
    int id = node->getNodeID();
    if (id < 0) id = _comm->getNodeIdByName(node->getName());
    int pid = (id >= 0) ? _comm->getNodePidByName(node->getName()) : -1;
    if (id >= 0 && pid > 0) {
      _comm->sendRequest(node, command);
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
  if (_rc_node->getState() != RCState::ERROR_ES) {
    _rc_node->setState(RCState::ERROR_ES);
    reportState(_rc_node);
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
  /*
  std::cerr << __FILE__ << ":" << __LINE__ << " RUNCONTROL>>GUI ("
      << node->getName().c_str() << "="
      << node->getState().getLabel() << ")" << std::endl;
  */
  RunControlMessage msg(RunControlMessage::LOCALNSM, nsm);
  msg.setCommand(RCCommand::STATE);
  try {
    _ui_comm->sendMessage(msg);
  } catch (const IOException& e) {
    B2DAQ::debug("%s:%d error=%s", __FILE__, __LINE__, e.what());
  }
  if (node != _rc_node && node->getState() == RCState::ERROR_ES) {
    _rc_node->setState(RCState::ERROR_ES);
    reportState(_rc_node);
  }
  return true;
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
