#include "RunControlCallback.hh"

#include <nsm/NSMCommunicator.hh>

extern "C" {
#include <belle2nsm.h>
}

#include <xml/NodeLoader.hh>

#include <system/Date.hh>

#include <util/Debugger.hh>

#include <cstring>

using namespace B2DAQ;

bool RunControlCallback::send(const RCCommand& command,
                              const State& state_success) throw()
{
  NSMCommunicator* comm = getCommunicator();
  RCState state_org = _rc_node->getState();
  bool is_success = true;
  std::vector<NSMNode*>& node_v(_node_system->getNodes());
  try {
    for (size_t index = 0; index < node_v.size(); index++) {
      NSMNode* node(node_v[index]);
      int id = node->getNodeID();
      if (id < 0) id = b2nsm_nodeid(node->getName().c_str());
      int pid = (id >= 0) ? b2nsm_nodepid(node->getName().c_str()) : -1;
      if (id >= 0 && pid > 0) {
        if (command != RCCommand::RECOVER) {
          comm->sendRequest(node, command);
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

bool RunControlCallback::ok() throw()
{
  NSMCommunicator* comm = getCommunicator();
  int id = comm->getMessage().getNodeId();
  NSMNode* node = getNodeByID(id);
  if (node == NULL) {
    std::string nodename = comm->getMessage().getNodeName();
    node = getNodeByName(nodename);
    if (node == NULL) {
      B2DAQ::debug("Unexcepted node id: ", id);
      return false;
    }
  }
  const char* datap = (const char*)(comm->getMessage().getData().c_str());
  node->setState(RCState(datap));
  RCState state_org = _rc_node->getState();
  RCState next;
  bool synchronized = false;
  std::vector<NSMNode*>& node_v(_node_system->getNodes());
  if (state_org.isTransaction()) {
    next = state_org.next();
    for (size_t i = 0; i < node_v.size(); i++) {
      synchronized = (next == node_v[i]->getState());
      if (!synchronized) break;
    }
  } else if (state_org == RCState::RECOVERING_RS) {
    synchronized = tryRecover(node);
    next = RCState::READY_S;
    for (size_t i = 0; i < node_v.size(); i++) {
      if (!synchronized) break;
      synchronized = (next == node_v[i]->getState());
    }
  } else {
    next = node->getState();
    synchronized = false;
    for (size_t i = 0; i < node_v.size(); i++) {
      synchronized = (next == node_v[i]->getState());
      if (!synchronized) break;
    }
  }
  reportState(node);
  if (synchronized && state_org != next) {
    _rc_node->setState(next);
    reportState(state_org);
  }
  return true;
}

bool RunControlCallback::error() throw()
{
  NSMCommunicator* comm = getCommunicator();
  int id = comm->getMessage().getNodeId();
  NSMNode* node = getNodeByID(id);
  if (node == NULL) {
    std::string nodename = comm->getMessage().getNodeName();
    node = getNodeByName(nodename);
    if (node == NULL) {
      B2DAQ::debug("Unexcepted node id: ", id);
      return false;
    }
  }
  node->setState(RCState::ERROR_ES);
  reportState(node);
  return true;
}

bool RunControlCallback::reportState(NSMNode* node) throw()
{
  NSMCommunicator* comm = getCommunicator();
  try {
    int pars[3] = {node->getIndex(), node->getConnection().getId(), node->getState().getId()};
    int npar = sizeof(pars) / sizeof(int);
    B2DAQ::debug("%s:%d, %d, %d", node->getName().c_str(), pars[0],  pars[1], pars[2]);
    comm->sendRequest(_master_node, RCCommand::STATE, npar, pars);
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to send message to %s", _master_node->getName().c_str());
    return false;
  }
  return true;
}

bool RunControlCallback::reportState(const State& state_org) throw()
{
  NSMCommunicator* comm = getCommunicator();
  if ((state_org == RCState::STARTING_TS && _rc_node->getState() == RCState::RUNNING_S) ||
      (state_org == RCState::RUNNING_S && _rc_node->getState() == RCState::STOPPING_TS)) {
    Date date;
    if (state_org == RCState::STARTING_TS) {
      _status->incrementRunNumber();
      _status->setStartTime(date.get());
      _status->setEndTime(0);
      B2DAQ::debug("Run #%d has started at %s", date.toString());
    } else {
      _status->setEndTime(date.get());
      _status->setStartTime(0);
      B2DAQ::debug("Run #%d has ended at %s", date.toString());
    }
    int pars[7];
    int npar = sizeof(pars) / sizeof(int);
    pars[0] = -1;
    pars[1] = _rc_node->getConnection().getId();
    pars[2] = _rc_node->getState().getId();
    unsigned long int run_no = _status->getRunNumber();
    unsigned long int t = date.get();
    memcpy(&pars[3], &run_no, sizeof(long int));
    memcpy(&pars[5], &t, sizeof(long int));
    try {
      comm->sendRequest(_master_node, RCCommand::STATE, npar, pars);
    } catch (const NSMHandlerException& e) {
      B2DAQ::debug("Failed to send message to %s", _master_node->getName().c_str());
      return false;
    }
  } else {
    return reportState(_rc_node);
  }
  return true;
}

bool RunControlCallback::tryRecover(NSMNode* node)
{
  NSMCommunicator* comm = getCommunicator();
  if (node->getState() == RCState::INITIAL_S) {
    comm->sendRequest(node, RCCommand::BOOT);
  } else if (node->getState() == RCState::CONFIGURED_S) {
    comm->sendRequest(node, RCCommand::LOAD);
  } else if (node->getState() == RCState::READY_S) {
    return true;
  } else if (node->getState() == RCState::PAUSED_S) {
    comm->sendRequest(node, RCCommand::STOP);
  } else if (node->getState() == RCState::RUNNING_S) {
    comm->sendRequest(node, RCCommand::STOP);
  } else if (node->getState() == RCState::ERROR_ES) {
    comm->sendRequest(node, RCCommand::RECOVER);
  }
  return false;
}

