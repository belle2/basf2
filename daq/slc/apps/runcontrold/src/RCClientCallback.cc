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
  _master->getNode()->setState(State::UNKNOWN);
}

RCClientCallback::~RCClientCallback() throw()
{

}

void RCClientCallback::init() throw()
{
  RCNSMCommunicator* com = new RCNSMCommunicator(getCommunicator());
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
    _master->unlock();
    return true;
  }
  if (nsm.getData() == node->getName()) {
    node->setState(State(nsm.getParam(0)));
    node->setConnection(Connection(nsm.getParam(1)));
  } else {
    node->setState(State(nsm.getData()));
    if (node->getState() == State::UNKNOWN) {
      LogFile::debug(std::string(nsm.getNodeName()) + " got unknown state : " + nsm.getData());
    }
  }
  if (node->getConnection() != Connection::ONLINE) {
    RCCommunicator* comm = _master->getClientCommunicator();
    std::string str = Belle2::form("Node %s got online (%s).", node->getName().c_str(),
                                   node->getState().getAlias());
    comm->sendLog(SystemLog(getCommunicator()->getNode()->getName(),
                            SystemLog::INFO, str));
    LogFile::info(str);
    node->setConnection(Connection::ONLINE);
  }
  RCSequencer::notify();
  bool synchronized = true;
  State state_org = _master->getNode()->getState();
  for (RCMaster::NSMNodeList::iterator it = _master->getNSMNodes().begin();
       it != _master->getNSMNodes().end(); it++) {
    if ((*it)->isUsed() && ((*it)->getState() == State::UNKNOWN ||
                            (*it)->getState() != node->getState())) {
      synchronized = false;
      break;
    }
  }
  State& state(node->getState());
  if (synchronized && state != State::UNKNOWN) {
    _master->getNode()->setState(state);
    if (state == State::RUNNING_S) {
      std::string str = Belle2::form("New run #%d has been started.",
                                     _master->getStatus()->getColdNumber());
      LogFile::info(str);
      _master->sendLogToMaster(SystemLog(getCommunicator()->getNode()->getName(),
                                         SystemLog::INFO, str));
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
  _master->getNode()->setState(State::ERROR_ES);
  std::string emsg = Belle2::form("%s got error (message = %s)",
                                  node->getName().c_str(), nsm.getData());
  nsm.setData(emsg);
  RunControlMessage msg(_master->getNode(), Command::ERROR, nsm);
  _master->sendMessageToMaster(msg);
  _master->sendStateToMaster(node);
  _master->sendStateToMaster(_master->getNode());
  _master->signal();
  _master->unlock();
  LogFile::error(emsg);
  return true;
}

void RCClientCallback::selfCheck() throw()
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
      if (!comm->isOnline(node) /*|| _master->getNode()->getState() == State::UNKNOWN)*/) {
        Connection connection_org = node->getConnection();
        node->setState(State::UNKNOWN);
        node->setConnection(Connection::OFFLINE);
        _master->getNode()->setState(State::ERROR_ES);
        if (!_requested_once || connection_org == Connection::ONLINE) {
          std::string str = Belle2::form("Node %s got offline", node->getName().c_str());
          comm->sendLog(SystemLog(getCommunicator()->getNode()->getName(),
                                  SystemLog::ERROR, str));
          LogFile::error(str);
          _master->sendStateToMaster(node);
          _master->sendStateToMaster(_master->getNode());
        }
      }
      State& state(node->getState());
      if (comm->isOnline(node) && state == State::UNKNOWN) {
        if (!comm->sendMessage(msg)) {
          node->setConnection(Connection::OFFLINE);
          node->setState(State::UNKNOWN);
          _master->getNode()->setState(State::ERROR_ES);
          _master->sendStateToMaster(node);
          _master->sendStateToMaster(_master->getNode());
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
  _requested_once = true;
  _master->unlock();
}
