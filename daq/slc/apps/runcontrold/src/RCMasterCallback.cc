
#include "daq/slc/apps/runcontrold/RCMasterCallback.h"

#include "daq/slc/apps/runcontrold/RCCommunicator.h"
#include "daq/slc/apps/runcontrold/RCMaster.h"
#include "daq/slc/apps/runcontrold/RCSequencer.h"
#include "daq/slc/apps/runcontrold/RCNSMCommunicator.h"
#include "daq/slc/apps/runcontrold/RCDatabaseManager.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/State.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>

#include <sstream>

using namespace Belle2;

RCMasterCallback::RCMasterCallback(NSMNode* node)
  : RCCallback(node)
{
}

RCMasterCallback::~RCMasterCallback() throw()
{

}

void RCMasterCallback::init() throw()
{
  RCNSMCommunicator* com = new RCNSMCommunicator(getNode(),
                                                 getCommunicator());
  _master->lock();
  _master->addMasterCommunicator(com);
  _master->unlock();
}

bool RCMasterCallback::distribute(Command command,
                                  int num0, int num1, int num2)
{
  _master->lock();
  std::string str = getMessage().getData();
  std::vector<std::string> str_v = Belle2::split(str, ' ');
  std::string name = (str_v.size() > 0) ? str_v[0] : "";
  NSMNode* node = _master->getNodeByName(name);
  RunControlMessage msg(node, command, num0, num1, num2);
  if (str_v.size() > 1) {
    msg.getMessage().setData(str_v[1]);
  }
  if (node != NULL) {
    RCCommunicator* comm = _master->getClientCommunicator();
    if (command == Command::LOAD &&
        num0 < 0 && node->getData() != NULL) {
      msg.getMessage().setParam(0, node->getData()->getConfigNumber());
    }
    try {
      if (!comm->isOnline(node)) {
        if (node->getConnection() == Connection::ONLINE) {
          node->setState(State::ERROR_ES);
          node->setConnection(Connection::OFFLINE);
          _master->sendStateToMaster(node);
        }
      } else {
        if (!comm->sendMessage(msg)) {
          _master->getNode()->setState(State::ERROR_ES);
        } else {
          State state = msg.getCommand().nextTState();
          if (state != State::UNKNOWN)
            node->setState(state);
        }
        _master->sendStateToMaster(node);
      }
    } catch (const NSMHandlerException& e) {
      setReply("NSM error");
      _master->unlock();
      return false;
    }
  } else {
    PThread(new RCSequencer(_master, msg));
  }
  _master->unlock();
  return true;
}

bool RCMasterCallback::boot() throw()
{
  return distribute(Command::BOOT, 0, 0, 0);
}

bool RCMasterCallback::load() throw()
{
  _master->lock();
  _master->getStatus()->update();
  int confno = (getMessage().getNParams() > 0) ? getMessage().getParam(0) : -1;
  confno = _master->getDBManager()->readConfigs(confno);
  _master->sendDataObjectToMaster(_master->getNode()->getName(),
                                  _master->getData());
  _master->sendDataObjectToMaster(_master->getConfig()->getClassName(),
                                  _master->getConfig());
  _master->sendDataObjectToMaster(_master->getStatus()->getClassName(),
                                  _master->getStatus());
  _master->unlock();
  //trigft();
  return distribute(Command::LOAD, -1, 0, 0);
}

bool RCMasterCallback::start() throw()
{
  NSMMessage& nsm(getMessage());
  int exp_no = nsm.getParam(0);
  int cold_no = nsm.getParam(1);
  int hot_no = 0;
  _master->lock();
  RunStatus* status = _master->getStatus();
  std::vector<std::string> str_v = Belle2::split(nsm.getData(), ' ');
  if (str_v.size() > 1) {
    status->setOperators(str_v[1]);
    std::stringstream ss;
    for (size_t i = 2; i < str_v.size(); i++) {
      std::string s = Belle2::replace(str_v[1], "\'", "");
      s = Belle2::replace(s, "\"", "");
      s = Belle2::replace(s, "\r\n", "<br/>");
      s = Belle2::replace(s, "\n", "<br/>");
      s = Belle2::replace(s, "\r", "<br/>");
      ss << s << " ";
    }
    status->setComment(ss.str());
  }
  if (exp_no < 0) exp_no = 0;
  if (cold_no < 0) cold_no = status->getColdNumber() + 1;
  status->setExpNumber(exp_no);
  status->setColdNumber(cold_no);
  status->setHotNumber(hot_no);
  status->setStartTime(Time().getSecond());
  status->setEndTime(-1);
  _master->getDBManager()->writeStatus();
  _master->sendDataObjectToMaster(status->getClassName(), status);
  _master->unlock();
  return distribute(Command::START, exp_no, cold_no, hot_no);
}

bool RCMasterCallback::stop() throw()
{
  _master->lock();
  RunStatus* status = _master->getStatus();
  status ->setEndTime(Time().getSecond());
  _master->getDBManager()->writeStatus();
  _master->sendDataObjectToMaster(status->getClassName(), status);
  _master->unlock();
  return distribute(Command::STOP, 0, 0, 0);
}

bool RCMasterCallback::trigft() throw()
{
  _master->lock();
  RCCommunicator* comm = _master->getClientCommunicator();
  for (RCMaster::NSMNodeList::iterator it = _master->getNSMNodes().begin();
       it != _master->getNSMNodes().end(); it++) {
    NSMNode* node = *it;
    if (!comm->isOnline(node)) {
      if (node->getConnection() == Connection::ONLINE) {
        node->setState(State::ERROR_ES);
        node->setConnection(Connection::OFFLINE);
        _master->sendStateToMaster(node);
      }
    } else if (node->getData() != NULL && node->getData()->hasObject("ftsw")) {
      DataObject* ftsw = node->getData()->getObject("ftsw");
      if (ftsw->hasValue("trigger_type") && ftsw->hasValue("dummy_rate") &&
          ftsw->hasValue("trigger_limit")) {
        NSMMessage& nsm(getMessage());
        if (nsm.getNParams() > 2 && (int)nsm.getParam(0) >= 0) {
          ftsw->setInt("trigger_type", nsm.getParam(0));
          ftsw->setInt("dummy_rate", nsm.getParam(1));
          ftsw->setInt("trigger_limit", nsm.getParam(2));
          ftsw->setConfigNumber(ftsw->getConfigNumber() + 1);
          RunConfig* run_config =  _master->getConfig();
          RunStatus* run_status =  _master->getStatus();
          run_config->update();
          int confno = run_config->getConfigNumber() + 1;
          run_config->setConfigNumber(confno);
          run_status->setRunConfig(confno);
          _master->getDBManager()->writeConfigs();
        }
        nsm.setNParams(3);
        nsm.setParam(0, ftsw->getInt("trigger_type"));
        nsm.setParam(1, ftsw->getInt("dummy_rate"));
        nsm.setParam(2, ftsw->getInt("trigger_limit"));
        RunControlMessage msg(node, Command::TRIGFT, nsm);
        try {
          comm->sendMessage(msg);
        } catch (const NSMHandlerException& e) {
          setReply("NSM error");
          _master->unlock();
          return false;
        }
      }
    }
  }
  _master->unlock();
  return true;
}

bool RCMasterCallback::recover() throw()
{
  return distribute(Command::RECOVER, 0, 0, 0);
}

bool RCMasterCallback::abort() throw()
{
  return distribute(Command::ABORT, 0, 0, 0);
}

bool RCMasterCallback::pause() throw()
{
  return distribute(Command::PAUSE, 0, 0, 0);
}

bool RCMasterCallback::resume() throw()
{
  return distribute(Command::RESUME, 0, 0, 0);
}

