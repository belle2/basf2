#include "daq/slc/apps/hvcontrold/HVMasterCallback.h"

#include "daq/slc/apps/hvcontrold/HVCommand.h"
#include "daq/slc/apps/hvcontrold/HVControlMaster.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>

#include <sstream>
#include <iostream>

using namespace Belle2;

HVMasterCallback::HVMasterCallback(NSMNode* node, HVControlMaster* master)
throw() : HVCallback(node), _master(master)
{
}

bool HVMasterCallback::perform(NSMMessage& msg)
throw(NSMHandlerException)
{
  HVCommand cmd(msg.getRequestName());
  LogFile::debug(cmd.getLabel());
  setReply("");
  if (cmd == Command::OK) {
    return ok();
  } else if (cmd == Command::ERROR) {
    return error();
  }
  HVState state(_node->getState());
  LogFile::debug(state.getLabel());
  bool result = true;
  NSMCommunicator* com = getCommunicator();
  if (state.isError()) {
    if (cmd == HVCommand::RECOVER) {
      result = recover();
    }
  } else if (state.isOff()) {
    if (cmd == HVCommand::CONFIGURE) {
      result = config();
    } else if (cmd == HVCommand::SAVE) {
      result = save();
    } else if (cmd == HVCommand::TURNON) {
      result = turnon();
    }
  } else if (state.isOn()) {
    if (cmd == HVCommand::RAMPUP) {
      result = rampup();
    } else if (cmd == HVCommand::RAMPDOWN) {
      result = rampdown();
    } else if (cmd == HVCommand::TURNOFF) {
      result = turnoff();
    } else if (cmd == HVCommand::PEAK) {
      result = peak();
    } else if (cmd == HVCommand::STANDBY) {
      result = standby();
    } else if (cmd == HVCommand::STANDBY2) {
      result = standby2();
    } else if (cmd == HVCommand::STANDBY3) {
      result = standby3();
    }
  } else if (cmd == Command::STATECHECK) {
    com->replyOK(_node, "");
    return true;
  }
  if (result) {
    com->replyOK(_node, _reply);
  } else {
    _node->setState(State::ERROR_ES);
    com->replyError(_reply);
  }
  return result;
}

void HVMasterCallback::init() throw()
{
  _comm = getCommunicator();
  for (HVNSMDataList::iterator it = _master->getDataList().begin();
       it != _master->getDataList().end(); it++) {
    NSMData* data = it->second;
    try {
      data->open(_comm);
    } catch (const NSMHandlerException& e) {}
  }
}

void HVMasterCallback::selfCheck() throw()
{
  bool is_transition = false;
  bool is_error = false;
  bool is_peak = true;
  bool is_off = true;
  for (HVNSMDataList::iterator it = _master->getDataList().begin();
       it != _master->getDataList().end(); it++) {
    NSMData* data = it->second;
    if (!data->isAvailable()) {
      try {
        data->open(_comm);
      } catch (const NSMHandlerException& e) {}
    }
    if (data->isAvailable()) {
      std::string nodename = it->first;
      HVNodeInfo& info(_master->getNodeInfo(nodename));
      info.loadStatus((hv_status*)data->get());
      HVState state = info.getNode()->getState().getId();
      LogFile::debug("state (%s) = %s", nodename.c_str(), state.getLabel());
      if (state != HVState::PEAK_S) is_peak = false;
      if (state != HVState::OFF_S) is_off = false;
      if (state.isTransition()) is_transition = true;
      else if (state.isError()) is_error = true;
    } else {
      is_peak = false;
      is_off = false;
    }
  }
  if (is_error) _node->setState(HVState::ERROR_ES);
  else if (is_transition) _node->setState(HVState::TRANSITION_TS);
  else if (is_peak) _node->setState(HVState::PEAK_S);
  else if (is_off) _node->setState(HVState::OFF_S);
  else _node->setState(HVState::STABLE_S);
  LogFile::debug("state (%s) = %s", _node->getName().c_str(), _node->getState().getLabel());
}

bool HVMasterCallback::ok() throw()
{
  NSMMessage& nsm(getMessage());
  LogFile::debug("OK : state (%s) = %s", nsm.getNodeName(), nsm.getData());
  return true;
}

bool HVMasterCallback::error() throw()
{
  NSMMessage& nsm(getMessage());
  LogFile::debug("Error : state (%s) = %s", nsm.getNodeName(), nsm.getData());
  return true;
}

bool HVMasterCallback::turnon() throw()
{
  for (HVNodeNameList::iterator it = _master->getNodeNameList().begin();
       it != _master->getNodeNameList().end(); it++) {
    std::string id = *it;
    HVNodeInfo& info(_master->getNodeInfo(id));
    NSMNode* node = info.getNode();
    HVState state = node->getState().getId();
    if (_comm->isConnected(node) && state.isOff()) {
      _node->setState(HVState::TRANSITION_TS);
      node->setState(HVState::TRANSITION_TS);
      _comm->sendRequest(node, HVCommand::TURNON);
    }
  }
  return true;
}

bool HVMasterCallback::turnoff() throw()
{
  for (HVNodeNameList::iterator it = _master->getNodeNameList().begin();
       it != _master->getNodeNameList().end(); it++) {
    std::string id = *it;
    HVNodeInfo& info(_master->getNodeInfo(id));
    NSMNode* node = info.getNode();
    HVState state = node->getState().getId();
    if (_comm->isConnected(node) && !state.isOff()) {
      _node->setState(HVState::TRANSITION_TS);
      node->setState(HVState::TRANSITION_TS);
      _comm->sendRequest(node, HVCommand::TURNOFF);
    }
  }
  return true;
}

bool HVMasterCallback::rampup() throw()
{
  for (HVNodeNameList::iterator it = _master->getNodeNameList().begin();
       it != _master->getNodeNameList().end(); it++) {
    std::string id = *it;
    HVNodeInfo& info(_master->getNodeInfo(id));
    NSMNode* node = info.getNode();
    HVState state = node->getState().getId();
    if (_comm->isConnected(node) && !state.isError()) {
      _node->setState(HVState::TRANSITION_TS);
      node->setState(HVState::TRANSITION_TS);
      _comm->sendRequest(node, HVCommand::RAMPUP);
    }
  }
  return true;
}

bool HVMasterCallback::rampdown() throw()
{
  for (HVNodeNameList::iterator it = _master->getNodeNameList().begin();
       it != _master->getNodeNameList().end(); it++) {
    std::string id = *it;
    HVNodeInfo& info(_master->getNodeInfo(id));
    NSMNode* node = info.getNode();
    HVState state = node->getState().getId();
    if (_comm->isConnected(node) && !state.isError()) {
      _node->setState(HVState::TRANSITION_TS);
      node->setState(HVState::TRANSITION_TS);
      _comm->sendRequest(node, HVCommand::RAMPDOWN);
    }
  }
  return true;
}

bool HVMasterCallback::peak() throw()
{
  for (HVNodeNameList::iterator it = _master->getNodeNameList().begin();
       it != _master->getNodeNameList().end(); it++) {
    std::string id = *it;
    HVNodeInfo& info(_master->getNodeInfo(id));
    NSMNode* node = info.getNode();
    HVState state = node->getState().getId();
    if (_comm->isConnected(node) && !state.isError()) {
      _node->setState(HVState::TRANSITION_TS);
      node->setState(HVState::TRANSITION_TS);
      _comm->sendRequest(node, HVCommand::PEAK);
    }
  }
  return true;
}

bool HVMasterCallback::standby() throw()
{
  for (HVNodeNameList::iterator it = _master->getNodeNameList().begin();
       it != _master->getNodeNameList().end(); it++) {
    std::string id = *it;
    HVNodeInfo& info(_master->getNodeInfo(id));
    NSMNode* node = info.getNode();
    HVState state = node->getState().getId();
    if (_comm->isConnected(node) && !state.isOff()) {
      _node->setState(HVState::TRANSITION_TS);
      node->setState(HVState::TRANSITION_TS);
      _comm->sendRequest(node, HVCommand::STANDBY);
    }
  }
  return true;
}

bool HVMasterCallback::standby2() throw()
{
  for (HVNodeNameList::iterator it = _master->getNodeNameList().begin();
       it != _master->getNodeNameList().end(); it++) {
    std::string id = *it;
    HVNodeInfo& info(_master->getNodeInfo(id));
    NSMNode* node = info.getNode();
    HVState state = node->getState().getId();
    if (_comm->isConnected(node) && !state.isOff()) {
      _node->setState(HVState::TRANSITION_TS);
      node->setState(HVState::TRANSITION_TS);
      _comm->sendRequest(node, HVCommand::STANDBY2);
    }
  }
  return true;
}

bool HVMasterCallback::standby3() throw()
{
  for (HVNodeNameList::iterator it = _master->getNodeNameList().begin();
       it != _master->getNodeNameList().end(); it++) {
    std::string id = *it;
    HVNodeInfo& info(_master->getNodeInfo(id));
    NSMNode* node = info.getNode();
    HVState state = node->getState().getId();
    if (_comm->isConnected(node) && !state.isOff()) {
      node->setState(HVState::TRANSITION_TS);
      _comm->sendRequest(node, HVCommand::STANDBY3);
    }
  }
  return true;
}

bool HVMasterCallback::config() throw()
{
  std::vector<std::string> nodename_v = Belle2::split(getMessage().getData(), ' ');
  std::map<std::string, int> id_m;
  const size_t nnodes = getMessage().getParam(0);
  if (nnodes <= nodename_v.size()) {
    for (size_t i = 0; i < nnodes; i++) {
      int id = getMessage().getParam(i + 1);
      std::string nodename = nodename_v[i];
      id_m.insert(std::map<std::string, int>::value_type(nodename, id));
      if (_master->hasNode(nodename) && id > 0) {
        _master->getNodeInfo(nodename).setConfigId(id);
      }
    }
  }
  DBInterface* db = _master->getDB();
  if (db != NULL) {
    try {
      db->connect();
      for (HVNodeNameList::iterator it = _master->getNodeNameList().begin();
           it != _master->getNodeNameList().end(); it++) {
        std::string nodename = *it;
        DBInterface* db = _master->getDB();
        HVNodeInfo& info(_master->getNodeInfo(nodename));
        if (id_m.find(nodename) == id_m.end() || id_m[nodename] > 0) {
          if (info.load(db) == 0) {
            int id = info.save(db);
            LogFile::debug("No entry with configid = %d. saved current config as %d",
                           info.getConfigId(), id);
          }
        }
      }
    } catch (const DBHandlerException& e) {
      LogFile::debug("DB access error:%s", e.what());
      setReply("DB access error");
      db->close();
      return false;
    }
    db->close();
  }
  try {
    for (HVNodeNameList::iterator it = _master->getNodeNameList().begin();
         it != _master->getNodeNameList().end(); it++) {
      std::string nodename = *it;
      HVNodeInfo& info(_master->getNodeInfo(nodename));
      if (_comm->isConnected(info.getNode())) {
        int npar = 1;
        int pars = info.getConfigId();
        info.getNode()->setConnection(Connection::ONLINE);
        if (id_m.find(nodename) == id_m.end() || id_m[nodename] >= 0) {
          _comm->sendRequest(info.getNode(), HVCommand::LOAD, npar, &pars);
        }
      } else {
        info.getNode()->setConnection(Connection::OFFLINE);
      }
    }
  } catch (const NSMHandlerException& e) {
    LogFile::debug("NSM access error:%s", e.what());
  }
  return true;
}

bool HVMasterCallback::save() throw()
{
  std::stringstream ss;
  DBInterface* db = _master->getDB();
  if (db != NULL) {
    try {
      db->connect();
      for (HVNodeNameList::iterator it = _master->getNodeNameList().begin();
           it != _master->getNodeNameList().end(); it++) {
        std::string id = *it;
        HVNodeInfo& info(_master->getNodeInfo(id));
        info.create(db);
        int configid = info.save(db);
        ss << configid << " ";
      }
    } catch (const DBHandlerException& e) {
      LogFile::debug("DB access error:%s", e.what());
      setReply("DB access error");
      return false;
    }
    db->close();
  }
  setReply(ss.str());
  return true;
}
