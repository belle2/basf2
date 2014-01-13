#include "daq/slc/apps/runcontrold/RCClientCallback.h"

#include "daq/slc/apps/runcontrold/RCMaster.h"
#include "daq/slc/apps/runcontrold/RCCommunicator.h"
#include "daq/slc/apps/runcontrold/RCNSMCommunicator.h"
#include "daq/slc/apps/runcontrold/RCSequencer.h"
#include "daq/slc/apps/runcontrold/RunControlMessage.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

RCClientCallback::RCClientCallback(NSMNode* node, RCMaster* master)
  : RCCallback(node), _master(master)
{
  _requested_once = false;
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
  if (node == NULL) {
    LogFile::debug("Got OK from unknown node (id=%d) ", id);
    return true;
  }
  node->setState(State(nsm.getData()));
  if (node->getConnection() != Connection::ONLINE) {
    RCCommunicator* comm = _master->getClientCommunicator();
    comm->sendLog(SystemLog(getCommunicator()->getNode()->getName(), SystemLog::INFO,
                            Belle2::form("Node %s connected.", node->getName().c_str())));
    node->setConnection(Connection::ONLINE);
  }
  RCSequencer::notify();
  bool synchronized = true;
  State state_org = _master->getNode()->getState();
  for (RCMaster::NSMNodeList::iterator it = _master->getNSMNodes().begin();
       it != _master->getNSMNodes().end(); it++) {
    if ((*it)->isUsed() && (*it)->getState() != node->getState()) {
      synchronized = false;
      break;
    }
  }
  if (synchronized) {
    State& state(node->getState());
    _master->getNode()->setState(state);
    if (state == State::RUNNING_S) {
      _master->sendLogToMaster(SystemLog(getCommunicator()->getNode()->getName(), SystemLog::INFO,
                                         Belle2::form("New run %d was started.",
                                                      _master->getStatus()->getColdNumber())));
    }
  }
  _master->sendStateToMaster(node);
  _master->sendStateToMaster(_master->getNode());
  _master->getStatus()->update();
  _master->signal();
  _master->unlock();
  return true;
}

bool RCClientCallback::error() throw()
{
  NSMMessage& nsm(getMessage());
  int id = nsm.getNodeId();
  _master->unlock();
  NSMNode* node = _master->findNode(id, nsm);
  if (node == NULL) {
    LogFile::debug("Got OK from unknown node (id=%d) ", id);
    return true;
  }
  node->setState(State::ERROR_ES);
  node->setConnection(Connection::ONLINE);
  _master->getStatus()->update();
  RunControlMessage msg(node, Command::ERROR, nsm);
  _master->sendMessageToMaster(msg);
  _master->signal();
  _master->unlock();
  LogFile::error("%s got error (message = %s)", node->getName().c_str(), nsm.getData().c_str());
  return true;
}

void RCClientCallback::selfCheck() throw(NSMHandlerException)
{
  _master->lock();
  RCCommunicator* comm = _master->getClientCommunicator();
  RunControlMessage msg(NULL, Command::STATECHECK);
  for (RCMaster::NSMNodeList::iterator it = _master->getNSMNodes().begin();
       it != _master->getNSMNodes().end(); it++) {
    NSMNode* node = *it;
    if (!node->isUsed()) continue;
    msg.setNode(node);
    try {
      if (!comm->isOnline(node)) {
        if (node->getConnection() == Connection::ONLINE) {
          node->setState(State::UNKNOWN);
          node->setConnection(Connection::OFFLINE);
          comm->sendLog(SystemLog(getCommunicator()->getNode()->getName(), SystemLog::ERROR,
                                  Belle2::form("Node %s got down.", node->getName().c_str())));
          _master->getNode()->setState(State::ERROR_ES);
          _master->sendStateToMaster(node);
        }
      }
      State& state(node->getState());
      if (state == State::UNKNOWN) {
        if (!comm->sendMessage(msg)) {
          node->setConnection(Connection::OFFLINE);
          node->setState(State::UNKNOWN);
          _master->getNode()->setState(State::ERROR_ES);
          _master->sendStateToMaster(node);
        }
      }
    } catch (const IOException& e) {
      LogFile::debug("IO error: %s", e.what());
      _master->getNode()->setState(State::ERROR_ES);
      _master->sendStateToMaster(_master->getNode());
      _master->unlock();
      return;
    }
  }
  _master->sendStateToMaster(_master->getNode());
  _requested_once = true;
  _master->unlock();
}
