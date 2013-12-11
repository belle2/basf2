#include "daq/slc/apps/runcontrold/RCSequencer.h"

#include "daq/slc/apps/runcontrold/RCMaster.h"
#include "daq/slc/apps/runcontrold/RCCommunicator.h"
#include "daq/slc/base/Debugger.h"

#include <unistd.h>

using namespace Belle2;

RCSequencer::RCSequencerList RCSequencer::__seq_l;
Mutex RCSequencer::__mutex;
Cond RCSequencer::__cond;
bool RCSequencer::__killed;

void RCSequencer::notify(bool killed)
{
  __mutex.lock();
  __killed = killed;
  __cond.broadcast();
  __mutex.unlock();
}

void RCSequencer::init()
{
  __mutex = Mutex();
  __cond = Cond();
}

RCSequencer::RCSequencer(RCMaster* master,
                         RunControlMessage msg,
                         bool synchronized)
  : _master(master), _msg(msg), _synchronized(synchronized)
{
}

RCSequencer::~RCSequencer()
{
}

void RCSequencer::run() throw()
{
  __mutex.lock();
  RCCommunicator* comm = _master->getClientCommunicator();
  RCCommunicator* master_comm = _master->getMasterCommunicator();
  try {
    int num0 = _msg.getMessage().getParam(0);
    for (RCMaster::NSMNodeList::iterator it = _master->getNSMNodes().begin();
         it != _master->getNSMNodes().end(); it++) {
      NSMNode* node = *it;
      _msg.setNode(node);
      _master->lock();
      if (_msg.getCommand() == Command::LOAD &&
          num0 < 0 && node->getData() != NULL) {
        _msg.getMessage().setParam(0, node->getData()->getConfigNumber());
      }
      if (node->isUsed()) {
        if (!comm->isOnline(node)) {
          if (node->getConnection() == Connection::ONLINE) {
            node->setState(State::ERROR_ES);
            node->setConnection(Connection::OFFLINE);
            master_comm->sendState(node);
            _master->getNode()->setState(State::ERROR_ES);
            master_comm->sendState(_master->getNode());

          }
        } else if (comm->sendMessage(_msg)) {
        }
        _master->unlock();
        if (_synchronized && !_master->isSynchronized(node)) {
          __cond.wait(__mutex);
        }
      } else {
        _master->unlock();
      }
    }
  } catch (const NSMHandlerException& e) {
    //setReply("NSM error");
    _master->unlock();
  }
  __mutex.unlock();
  _master->unlock();
  return;
}
