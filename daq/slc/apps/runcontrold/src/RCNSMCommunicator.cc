#include "daq/slc/apps/runcontrold/RCNSMCommunicator.h"

using namespace Belle2;

RCNSMCommunicator::RCNSMCommunicator(NSMNode* rc_node, NSMCommunicator* comm)
  : _rc_node(rc_node), _comm(comm)
{

}

RCNSMCommunicator::~RCNSMCommunicator() throw()
{

}

bool RCNSMCommunicator::sendMessage(const RunControlMessage& msg) throw()
{
  _nsm_mutex.lock();
  try {
    _comm->sendRequest(msg.getNode(), msg.getCommand(), msg.getMessage().getNParams(),
                       (unsigned int*)msg.getMessage().getParams(), msg.getMessage().getData());
  } catch (const NSMHandlerException& e) {
    ((NSMNode*)msg.getNode())->setConnection(Connection::OFFLINE);
    _nsm_mutex.unlock();
    return false;
  }
  _nsm_mutex.unlock();
  return true;
}

bool RCNSMCommunicator::sendState(NSMNode* node) throw()
{
  _nsm_mutex.lock();
  try {
    unsigned int pars[2];
    int npar = sizeof(pars) / sizeof(int);
    pars[0] = node->getState().getId();
    pars[1] = node->getConnection().getId();
    _comm->sendRequest(_rc_node, Command::STATE, npar, pars, node->getName());
  } catch (const NSMHandlerException& e) {
    _rc_node->setConnection(Connection::OFFLINE);
    _nsm_mutex.unlock();
    return false;
  }
  _nsm_mutex.unlock();
  return true;
}

bool RCNSMCommunicator::sendDataObject(const std::string& name,
                                       DataObject* data) throw()
{
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
  return true;
}

bool RCNSMCommunicator::isOnline(NSMNode* node) throw()
{
  _nsm_mutex.lock();
  bool is_online = _comm->getNodeIdByName(node->getName()) > 0 &&
                   _comm->getNodePidByName(node->getName()) > 0;
  _nsm_mutex.unlock();
  return is_online;
}
