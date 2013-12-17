#include "daq/slc/apps/runcontrold/RCClientCallback.h"

#include "daq/slc/apps/runcontrold/RCMaster.h"
#include "daq/slc/apps/runcontrold/RCCommunicator.h"
#include "daq/slc/apps/runcontrold/RCNSMCommunicator.h"
#include "daq/slc/apps/runcontrold/RCSequencer.h"
#include "daq/slc/apps/runcontrold/RunControlMessage.h"

#include <daq/slc/base/Debugger.h>

using namespace Belle2;

RCClientCallback::RCClientCallback(NSMNode* node, RCMaster* master)
  : RCCallback(node), _master(master)
{

}

RCClientCallback::~RCClientCallback() throw()
{

}

void RCClientCallback::init() throw()
{
  RCNSMCommunicator* com = new RCNSMCommunicator(getNode(),
                                                 getCommunicator());
  _master->lock();
  _master->setClientCommunicator(com);
  _master->unlock();
}

bool RCClientCallback::ok() throw()
{
  NSMMessage& nsm(getMessage());
  int id = nsm.getNodeId();
  _master->lock();
  NSMNode* node = _master->findNode(id, nsm);
  node->setState(State(nsm.getData()));
  node->setConnection(Connection::ONLINE);
  RCSequencer::notify();
  RCCommunicator* comm = _master->getMasterCommunicator();
  bool synchronized = true;
  bool iserror = false;
  State state_low = State::RUNNING_S;;
  State state_org = _master->getNode()->getState();
  for (RCMaster::NSMNodeList::iterator it = _master->getNSMNodes().begin();
       it != _master->getNSMNodes().end(); it++) {
    if ((*it)->isUsed() && (*it)->getState() != node->getState())
      synchronized = false;
    iserror |= node->getState().isError();
    if (state_low.getId() > node->getState().getId()) {
      state_low = node->getState();
    }
  }
  if (synchronized) {
    _master->getNode()->setState(node->getState());
  }
  if (state_org.isError() && !iserror) {
    _master->getNode()->setState(state_low);
  }
  bool result = (comm != NULL) ? comm->sendState(node) : true;
  if (comm != NULL) comm->sendState(_master->getNode());
  _master->getStatus()->update();
  _master->signal();
  _master->unlock();
  return result;
}

bool RCClientCallback::error() throw()
{
  NSMMessage& nsm(getMessage());
  int id = nsm.getNodeId();
  _master->unlock();
  NSMNode* node = _master->findNode(id, nsm);
  node->setState(State::ERROR_ES);
  node->setConnection(Connection::ONLINE);
  _master->getStatus()->update();
  RCCommunicator* comm = _master->getMasterCommunicator();
  RunControlMessage msg(node, Command::ERROR, nsm);
  bool result = (comm != NULL) ? comm->sendMessage(msg) : true;
  _master->signal();
  _master->unlock();
  Belle2::debug("%s:%d error()", __FILE__, __LINE__);
  return result;
}

void RCClientCallback::selfCheck() throw(NSMHandlerException)
{
  _master->lock();
  if (_master->getNode()->getState() == State::RUNNING_S) {
    _master->unlock();
    return;
  }
  RCCommunicator* comm = _master->getClientCommunicator();
  RunControlMessage msg(NULL, Command::STATECHECK);
  RCCommunicator* master_comm = _master->getMasterCommunicator();
  for (RCMaster::NSMNodeList::iterator it = _master->getNSMNodes().begin();
       it != _master->getNSMNodes().end(); it++) {
    NSMNode* node = *it;
    msg.setNode(node);
    try {
      if (!comm->isOnline(node)) {
        if (node->getConnection() == Connection::ONLINE) {
          node->setState(State::ERROR_ES);
          node->setConnection(Connection::OFFLINE);
          _master->getNode()->setState(State::ERROR_ES);
          if (master_comm != NULL) {
            master_comm->sendState(node);
            master_comm->sendState(_master->getNode());
          }
        }
      }
      if (!comm->sendMessage(msg)) {
        _master->getNode()->setState(State::ERROR_ES);
        if (master_comm != NULL) {
          master_comm->sendState(node);
          master_comm->sendState(_master->getNode());
        }
      }
    } catch (const IOException& e) {
      Belle2::debug("%s:%d %s", __FILE__, __LINE__, e.what());
      _master->getNode()->setState(State::ERROR_ES);
      if (master_comm != NULL)
        master_comm->sendState(_master->getNode());
      _master->unlock();
      return;
    }
  }
  if (master_comm != NULL) {
    master_comm->sendState(_master->getNode());
  }
  _master->unlock();
}
