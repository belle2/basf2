#include "RunControlMessageManager.h"

#include "MessageBox.h"

#include "database/DBObjectLoader.h"

#include "system/Time.h"

#include "base/Date.h"
#include "base/Debugger.h"
#include "base/StringUtil.h"
#include "base/ConfigFile.h"

#include <unistd.h>
#include <fstream>

using namespace Belle2;

void RunControlMessageManager::run()
{
  std::vector<NSMNode*>& node_v(_node_system->getNodes());
  _ntry_recover = 0;
  downloadConfig(Command::SETPARAMS,
                 _node_system->getConfig()->getVersion());
  int index_seq = -1;
  Command cmd_seq;
  while (true) {
    RunControlMessage msg = MessageBox::get().pop();
    Command cmd = msg.getCommand();
    const NSMMessage& nsm(msg.getMessage());
    if (msg.getId() == RunControlMessage::GUI) {
      if (nsm.getParam(0) != (unsigned int) - 1) {
        NSMNode* node = _node_system->getNodes()[nsm.getParam(0)];
        if (cmd.isAvailable(node->getState())) {
          index_seq = -1;
          State state_next = getNextState(cmd);
          if (state_next != State::UNKNOWN)
            node->setState(state_next);
          reportState(node);
          send(node, cmd, nsm.getNParams(), nsm.getParams());
          uploadRunResult(cmd);
        }
      } else {
        if (!cmd.isAvailable(_rc_node->getState())) continue;
        index_seq = 0;
        cmd_seq = cmd;
        State state_next = getNextState(cmd);
        if (state_next != State::UNKNOWN) {
          _rc_node->setState(state_next);
        }
        downloadConfig(cmd, nsm.getParam(1));
        uploadRunConfig(cmd, nsm.getData());
        uploadRunResult(cmd);
        if (cmd == Command::STATECHECK) {
          for (size_t i = 0; i < node_v.size(); i++) {
            reportState(node_v[i]);
          }
          reportRCStatus();
          continue;
        } else {
          for (size_t i = 0; i < node_v.size(); i++) {
            State state_org = node_v[i]->getState();
            node_v[i]->setState(_rc_node->getState());
            reportState(node_v[i]);
          }
        }
        index_seq = distribute(index_seq, cmd_seq);
        reportRCStatus();
      }
    } else if (msg.getId() == RunControlMessage::LOCALNSM) {
      if (index_seq < 0 && cmd == Command::STATECHECK &&
          _rc_node->getState() != State::RUNNING_S) {
        for (size_t i = 0; i < node_v.size(); i++) {
          if (node_v[i]->isUsed()) send(node_v[i], cmd);
        }
      } else {
        int id = nsm.getNodeId();
        NSMNode* node = findNode(id, nsm);
        if (node != NULL) {
          if (cmd == Command::OK) {
            node->setState(State(nsm.getData()));
            reportState(node);
            for (size_t i = 0; i < node_v.size(); i++) {
              if (node_v[i]->isUsed()) reportState(node_v[i]);
            }
            if (isSynchronized(node->getState(), node_v.size())) {
              _rc_node->setState(node->getState());
              reportState(_rc_node);
            }
            if (index_seq >= (int)node_v.size()) index_seq = -1;
            if (index_seq > 0) {
              State next_state = _rc_node->getState().next();
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

NSMNode* RunControlMessageManager::findNode(int id, const NSMMessage& msg) throw()
{
  NSMNode* node = getNodeByID(id);
  if (node == NULL) {
    const char* nodename = msg.getNodeName();
    if (nodename != NULL) {
      node = getNodeByName(nodename);
      if (node == NULL) {
        Belle2::debug("[DEBUG] Unexcepted node id: %d", id);
        return NULL;
      }
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
    send(node, Command::BOOT, 0, NULL);
  } else if (node->getState() == State::CONFIGURED_S) {
    send(node, Command::LOAD, 0, NULL);
  } else if (node->getState() == State::READY_S) {
    return true;
  } else if (node->getState() == State::PAUSED_S) {
    send(node, Command::STOP, 0, NULL);
  } else if (node->getState() == State::RUNNING_S) {
    send(node, Command::STOP, 0, NULL);
  } else if (node->getState() == State::ERROR_ES) {
    send(node, Command::ABORT, 0, NULL);
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
        !(node_v[i]->getState() == state ||
          (state == State::CONFIGURED_S &&
           node_v[i]->getState() == State::READY_S))) return false;
  }
  return true;
}

int RunControlMessageManager::distribute(int index_seq, const Command& command) throw()
{
  std::vector<NSMNode*>& node_v(_node_system->getNodes());
  for (int i = index_seq; i < (int)node_v.size(); i++) {
    if (node_v[i]->isUsed()) {
      if ((command == Command::BOOT || command == Command::LOAD) &&
          i > index_seq && node_v[i]->isSynchronize()) return i;
      if (!send(node_v[i], command)) return -1;
    }
  }
  return (int)node_v.size();
}

bool RunControlMessageManager::send(NSMNode* node, const Command& command,
                                    int npar_in, const unsigned int* pars_in) throw()
{
  try {
    NSMMessage msg;
    if (npar_in > 1) {
      msg.setNParams(npar_in - 1);
      for (int i = 0; i < npar_in - 1; i++) {
        msg.setParam(i, pars_in[i + 1]);
      }
    }
    if (command == Command::START) {
      msg.setNParams(2);
      msg.setParam(0, _node_system->getStatus()->getExpNumber());
      msg.setParam(1, _node_system->getStatus()->getRunNumber());
    }
    RunControlMessage rcmsg(RunControlMessage::RUNCONTROLLER, command, msg);
    int id = _nsm_comm->sendMessage(node, rcmsg);
    if (getNodeByID(id) == NULL) addNode(id, node);
    if (id >= 0) {
      return true;
    } else {
      reportState(node);
    }
  } catch (const IOException& e) {
    Belle2::debug("[DEBUG] %s:%d : %s", __FILE__, __LINE__, e.what());
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
    Belle2::debug("[DEBUG] %s:%d error=%s", __FILE__, __LINE__, e.what());
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
    Belle2::debug("[DEBUG] %s:%d error=%s", __FILE__, __LINE__, e.what());
  }
  return true;
}

bool RunControlMessageManager::reportRCStatus() throw()
{
  NSMMessage nsm;
  nsm.setNParams(8);
  nsm.setParam(0, -1);
  nsm.setParam(1, _rc_node->getConnection().getId());
  nsm.setParam(2, _rc_node->getState().getId());
  nsm.setParam(3, _node_system->getConfig()->getVersion());
  nsm.setParam(4, _node_system->getStatus()->getExpNumber());
  nsm.setParam(5, _node_system->getStatus()->getRunNumber());
  nsm.setParam(6, _node_system->getStatus()->getStartTime());
  nsm.setParam(7, _node_system->getStatus()->getEndTime());
  nsm.setData(_node_system->getConfig()->getRunType() + "\n" +
              _node_system->getConfig()->getOperators());
  RunControlMessage msg(RunControlMessage::RUNCONTROLLER, nsm);
  msg.setCommand(Command::OK);
  try {
    _ui_comm->sendMessage(msg);
  } catch (const IOException& e) {
    Belle2::debug("[DEBUG] %s:%d error=%s", __FILE__, __LINE__, e.what());
  }
  return true;
}

void RunControlMessageManager::downloadConfig(const Command& cmd, int version) throw()
{
  if (cmd == Command::SETPARAMS) {
    _node_system->getConfig()->setVersion(version);
    std::vector<NSMNode*>& node_v(_node_system->getNodes());
    ConfigFile config;
    for (size_t i = 0; i < node_v.size(); i++) {
      NSMNode* node = node_v[i];
      if (node->getData() != NULL) {
        /*
        std::ofstream fout((config.get("CONFIG_FILE_DIR") + "/" + node->getName()).c_str());
        fout << node->getData()->getText() << std::endl;
        fout.close();
        */
      }
    }
    try {
      _dbconfig.readTable(version);
    } catch (const IOException& e) {
      Belle2::debug("[DEBUG] Error on loading system configuration.:%s", e.what());
    }
  }
}

void RunControlMessageManager::uploadRunConfig(const Command& cmd,
                                               const std::string& data) throw()
{
  if (cmd == Command::LOAD) {
    std::vector<std::string> str_v = Belle2::split(data, '\n');
    if (str_v.size() > 0 && str_v[0].size() > 0)
      _node_system->getConfig()->setRunType(str_v[0]);
    if (str_v.size() > 1 && str_v[1].size() > 0)
      _node_system->getConfig()->setOperators(str_v[1]);
  } else if (cmd == Command::START) {
    _node_system->getStatus()->incrementRunNumber();
    _node_system->getStatus()->setStartTime(Time().getSecond());
    _node_system->getStatus()->setEndTime(-1);
    DBObjectLoader handler(_db);
    try {
      handler.createTable(_node_system->getConfig());
    } catch (const IOException& e) {}
    _node_system->getConfig()->setRunNumber(_node_system->getStatus()->getRunNumber());
    _node_system->getConfig()->setStartTime(_node_system->getStatus()->getStartTime());
    try {
      std::vector<DataObject*> obj_v;
      obj_v.push_back(_node_system->getConfig());
      handler.writeTable(obj_v, 0);
    } catch (const IOException& e) {
      Belle2::debug("[DEBUG] failed to write new colum for run config:%s", e.what());
    }
  }
}

void RunControlMessageManager::uploadRunResult(const Command& cmd) throw()
{
  State state = _rc_node->getState();
  if (cmd == Command::STOP ||
      (cmd == Command::ABORT && state == State::RUNNING_S)) {
    DBObjectLoader handler(_db);
    _node_system->getStatus()->setEndTime(Time().getSecond());
    try {
      handler.createTable(_node_system->getStatus());
    } catch (const IOException& e) {}
    try {
      std::vector<DataObject*> obj_v;
      obj_v.push_back(_node_system->getStatus());
      handler.writeTable(obj_v, 0);
    } catch (const IOException& e) {
      Belle2::debug("[DEBUG] failed to write new colum for run config:%s", e.what());
    }
  }
}
