#include "daq/slc/apps/runcontrold/RCNSMCommunicator.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

RCNSMCommunicator::RCNSMCommunicator(NSMCommunicator* comm, NSMNode* node)
  : _rc_node(node), _comm(comm)
{

}

RCNSMCommunicator::~RCNSMCommunicator() throw()
{

}

bool RCNSMCommunicator::sendMessage(const RunControlMessage& msg) throw()
{
  _nsm_mutex.lock();
  try {
    LogFile::debug("send message via NSM (%s<<%s)", msg.getNode()->getName().c_str(),
                   msg.getCommand().getAlias());
    const NSMNode* node_to = (_rc_node != NULL) ? _rc_node : msg.getNode();
    _comm->sendRequest(node_to, msg.getCommand(), msg.getMessage().getNParams(),
                       (int*)msg.getMessage().getParams(), msg.getMessage().getData());
  } catch (const NSMHandlerException& e) {
    _nsm_mutex.unlock();
    return false;
  }
  _nsm_mutex.unlock();
  return true;
}

bool RCNSMCommunicator::sendState(NSMNode* node) throw()
{
  if (_rc_node == NULL ||
      node->getName() != _comm->getNode()->getName()) return true;
  _nsm_mutex.lock();
  try {
    LogFile::debug("send state via NSM to %s (%s=%s)",
                   _rc_node->getName().c_str(), node->getName().c_str(),
                   node->getState().getAlias());
    int pars[2];
    int npar = sizeof(pars) / sizeof(int);
    pars[0] = node->getState().getId();
    pars[1] = node->getConnection().getId();
    _comm->sendRequest(_rc_node, Command::OK, npar, pars, node->getName());
  } catch (const NSMHandlerException& e) {
    _rc_node->setConnection(Connection::OFFLINE);
    _nsm_mutex.unlock();
    return false;
  }
  _nsm_mutex.unlock();
  return true;
}

bool RCNSMCommunicator::sendDataObject(const std::string& /*name*/,
                                       DataObject* /*data*/) throw()
{
  if (_rc_node == NULL) return true;
  /*
  _nsm_mutex.lock();
  try {
    unsigned int pars[1];
    int npar = sizeof(pars) / sizeof(int);
    pars[0] = data->getConfigNumber();
    _comm->sendRequest(_rc_node, Command::DATA, npar, pars, name);
  } catch (const IOException& e) {
    _rc_node->setConnection(Connection::OFFLINE);
    _nsm_mutex.unlock();
    return false;
  }
  _nsm_mutex.unlock();
  */
  return true;
}

bool RCNSMCommunicator::isOnline(NSMNode* node) throw()
{
  _nsm_mutex.lock();
  bool is_online = _comm->getNodeIdByName(node->getName()) >= 0 &&
                   _comm->getNodePidByName(node->getName()) > 0;
  //LogFile::debug("%s : nodeid=%d, pid=%d (%s)", node->getName().c_str(),
  //     _comm->getNodeIdByName(node->getName()),
  //     _comm->getNodePidByName(node->getName()),
  //     ((is_online)?"ONLINE":"OFFLINE"));
  _nsm_mutex.unlock();
  return is_online;
}

void RCNSMCommunicator::sendLog(const SystemLog& log) throw()
{
  //if (_rc_node == NULL) return;
  //_comm->setRCNode(_rc_node);
  _comm->sendLog(log);
}
